#include "../../include/db/database_manager.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <stack>
#include <list>
#include "../../include/db/type_cast.h"
#include "../../include/db/col_val.h"
#include "../../include/btree/btree_iterator.hpp"
#include "../../include/db/record_manager.h"
#include "../../include/page/index_leaf_page.hpp"

void DatabaseManager::CreateDatabase(const std::string &db_name)
{
    std::strncpy(info_.db_name, db_name.c_str(), MAX_LENGTH_NAME);
    info_.num_table = 0;
    Close();
}

void DatabaseManager::Open(const std::string &db_name)
{
    if (std::strcmp(db_name.c_str(), info_.db_name) == 0) 
    { 
        is_open_ = true;
        return; 
    }
    if (is_open_)
    {
        Close();
        is_open_ = false;
    }
    std::ifstream ifs(DB_DIR + db_name + ".db", std::ios::in | std::ios::binary);
    if (!ifs.is_open()) { std::cout << "Database \"" << db_name << "\" doesn't exist!"; }
    else
    {
        ifs.read(reinterpret_cast<char *>(&info_), sizeof(info_));
        is_open_ = true;
        for (size_t i = 0; i < info_.num_table; ++i)
        {
            table_manager_.push_back(std::make_shared<TableManager>(info_.table_name[i]));
        }
    }
}

void DatabaseManager::Close()
{
    std::ofstream ofs(DB_DIR + info_.db_name + ".db", std::ios::out | std::ios::binary);
    ofs.write(reinterpret_cast<const char *>(&info_), sizeof(info_));
    is_open_ = false;
    // Close table
    for (size_t i = 0; i < info_.num_table; ++i)
    {
        table_manager_[i]->CloseTable();
    }
}

void DatabaseManager::CreateTable(const std::shared_ptr<TableHeader> table_header)
{
    if (!is_open_) 
    {
        std::cout << "Haven't choose a database!" << std::endl;
    }
    else 
    {
        for (size_t i = 0; i < info_.num_table; ++i)
        {
            if (std::strcpy(info_.table_name[i], table_header->table_name) == 0)
            {
                std::cout << "Table \"" << table_header->table_name << "\" has exist!" << std::endl;
                return;
            }
        }
        std::strncpy(info_.table_name[info_.num_table], table_header->table_name, MAX_LENGTH_NAME);
        ++info_.num_table;
        table_manager_.emplace_back(std::make_shared<TableManager>());
        table_manager_.back()->CreateTable(table_header);
    }
}

void DatabaseManager::InsertRow(const std::shared_ptr<InsertInfo> insert_info)
{
    std::shared_ptr<TableManager> table;
    for (auto &t : table_manager_)
    {
        if (t->table_name() == insert_info->table_name)
        {
            table = t;
            break;
        }
    }
    if (table == nullptr) 
    {
        std::cerr << "table " << insert_info->table_name << " not found!" << std::endl;
        return;
    }
    table->OpenTable(table->table_name());
    if (insert_info->field_name.empty())
    {
        if (insert_info->col_val.size() != table->number_of_column())
        {
            std::cerr << "column size not equal!" << std::endl;
            return;
        }
        for (size_t i = 0; i < table->number_of_column(); ++i)
        {
            if (!TypeCast::make_type_compatible(insert_info->col_val[i], table->column_type(i)))
            {
                std::cerr << "incompatible type!" << std::endl;
                return;
            }
            table->SetTempRecord(i, insert_info->col_val[i]);
        }
    }
    else
    {
        auto &field_map = insert_info->field_name;
        ColVal null_value;
        for (int i = 0; i < static_cast<int>(table->number_of_column()) - 1; ++i)
        {
            // insert_info don't have __rowid__, and we will set __rowid__ in InsertRecord()
            if (!TypeCast::make_type_compatible(insert_info->col_val[i], table->column_type(i)))
            {
                std::cerr << "incompatible type!" << std::endl;
                return;
            }
            if (field_map.find(table->column_name(i)) != field_map.end())   
            {
                table->SetTempRecord(i, insert_info->col_val[field_map[table->column_name(i)]]);
            }
            else
            {
                table->SetTempRecord(i, null_value);
            }
        }
    }
    table->InsertRecord();
}

void DatabaseManager::SelectTable(const std::shared_ptr<SelectInfo> select_info)
{
    // Construct a table map for founding table that does not exist quickly
    std::unordered_map<std::string, std::shared_ptr<TableManager>> table_map;
    for (const auto &item : select_info->tables)
    {
        table_map[item] = nullptr;
    }
    for (size_t i = 0; i < table_manager_.size(); ++i)
    {
        if (table_map.find(table_manager_[i]->table_name()) != table_map.end())
        {
            table_map[table_manager_[i]->table_name()] = table_manager_[i];
        }
    }
    for (const auto &item : table_map)
    {
        if (item.second == nullptr)
        {
            std::cout << "Error: no table named \"" << item.first << "\"!" << std::endl;
            return;
        }
    }

    // Get table manager vector
    std::vector<std::shared_ptr<TableManager>> tms;
    tms.reserve(select_info->tables.size());
    for (const auto &item : select_info->tables)
    {
        tms.push_back(table_map[item]);
    }
    // Get columns
    auto columns = select_info->columns;
    // Get where expression
    auto condition = select_info->where;

    // If condition is nullptr or has "or", cartisian product directly
    bool cartisian_product_directly = false;
    if (condition == nullptr)
    {
        cartisian_product_directly = true;
    }
    else
    {
        auto expr = condition;
        while (expr != nullptr)
        {
            if (expr->operator_type_ == Operator_Type::OR)
            {
                cartisian_product_directly = true;
            }
            expr = expr->next_expr_;
        }
    }
    if (cartisian_product_directly)
    {
        // Iterator records
        if (tms.size() == 1)
        {
            iterate_one_table(tms[0], columns, condition);
        }
        else
        {
            iterate_many_table(tms, columns, condition);
        }
        return;
    }
    // Else, use index to accelerate finding if it could
    // 1. converse reserve polish notation to normal then store in a list
    auto p = condition, q = p->next_expr_;
    std::stack<ExprNode *> expr_stack;
    while (p != nullptr)
    {
        q = p->next_expr_;
        auto node = new ExprNode(p->operator_type_, p->term_, nullptr);
        if (node->operator_type_ == Operator_Type::NONE)
        {
            expr_stack.push(node);
        }
        else
        {
            // unary
            if (node->operator_type_ > Operator_Type::UNARY_DELIMETER)
            {
                auto expr = expr_stack.top();
                expr_stack.pop();
                node->next_expr_ = expr;
                expr_stack.push(node);
            }
            // binary
            else
            {
                auto expr2 = expr_stack.top();
                expr_stack.pop();
                auto expr1 = expr_stack.top();
                expr_stack.pop();
                auto p = expr1;
                while (p->next_expr_ != nullptr) { p = p->next_expr_; }
                p->next_expr_ = node;
                node->next_expr_ = expr2;
                expr_stack.push(expr1);
            }
        }
        p = q;
    }
    auto header = expr_stack.top();
    expr_stack.pop();
    std::list<ExprNode *> ls;
    auto expr = header;
    while (expr != nullptr)
    {
        ls.push_back(expr);
        expr = expr->next_expr_;
    }
    // 2. find expression start(or and) -> Col -> = -> (+-) -> value -> end( or and) 
    std::vector<std::shared_ptr<TermExpr>> cols;
    std::vector<std::shared_ptr<TermExpr>> vals;
    auto it = ls.begin();
    while (it != ls.end())
    {
        if ((*it)->operator_type_ == Operator_Type::EQ)
        {
            // TODO: check condtion such as col = -3
            auto prev = it; --prev;
            auto pprev = prev; --pprev;
            auto next = it; ++next;
            auto nnext = next; nnext++;
            if (
                ((*prev)->operator_type_ == Operator_Type::NONE && (*prev)->term_->term_type_  == Term_Type::TERM_COL_REF) &&
                (pprev == ls.end() || (*pprev)->operator_type_ == Operator_Type::AND) &&
                ((*next)->operator_type_ == Operator_Type::NONE && (*next)->term_->term_type_ != Term_Type::TERM_COL_REF && (*next)->term_->term_type_ != Term_Type::TERM_NULL) &&
                (nnext == ls.end() || (*nnext)->operator_type_ == Operator_Type::AND)
                )
            {
                cols.emplace_back(std::make_shared<TermExpr>(*(*prev)->term_));
                vals.emplace_back(std::make_shared<TermExpr>(*(*next)->term_));
            }
        }
        ++it;
    }
    // free
    expr = header;
    while (expr != nullptr)
    {
        auto tmp = expr;
        expr = expr->next_expr_;
        delete tmp;
    }
    // Iterator records
    if (tms.size() == 1)
    {
        iterate_one_table(tms[0], columns, condition, cols, vals);
    }
    else
    {
        iterate_many_table(tms, columns, condition, cols, vals);
    }
}

void DatabaseManager::update_column2term(const std::shared_ptr<TableManager> &tm,
                                         BTreeIterator<VariantPage> &btit, 
                                         RecordManager &rm,
                                         std::unordered_map<std::string, std::shared_ptr<TermExpr>> &column2term)
{
    column2term.clear();
    // Initialize each record manager
    rm.Open(*btit, false);
    for (size_t i = 0; i < tm->number_of_column(); ++i)
    {
        rm.Seek(tm->column_offset(i));
        size_t col_length = tm->column_length(i);
        switch (tm->column_type(i))
        {
        case Col_Type::COL_TYPE_INT:
        {
            int val;
            rm.Read(&val, col_length);
            auto term = std::make_shared<TermExpr>(val);
            column2term.insert({tm->column_name(i), term});
            break;
        }
        case Col_Type::COL_TYPE_DOUBLE:
        {
            double val;
            rm.Read(&val, col_length);
            auto term = std::make_shared<TermExpr>(val);
            column2term.insert({tm->column_name(i), term});
            break;
        }
        case Col_Type::COL_TYPE_BOOL:
        {
            bool val;
            rm.Read(&val, col_length);
            auto term = std::make_shared<TermExpr>(val);
            column2term.insert({tm->column_name(i), term});
            break;
        }
        case Col_Type::COL_TYPE_DATE:
        {
            Date val;
            rm.Read(&val, col_length);
            auto term = std::make_shared<TermExpr>(val);
            column2term.insert({tm->column_name(i), term});
            break;
        }
        case Col_Type::COL_TYPE_CHAR:
        case Col_Type::COL_TYPE_VARCHAR:
        {
            char *val = new char[col_length + 1];
            val[col_length] = '\0';
            rm.Read(val, col_length);
            auto term = std::make_shared<TermExpr>(val);
            column2term.insert({tm->column_name(i), term});
            delete[] val;
            break;
        }
        case Col_Type::COL_TYPE_NULL:
        {
            auto term = std::make_shared<TermExpr>();
            column2term.insert({tm->column_name(i), term});
        }
        default:
            break;
        }
    }
}

void DatabaseManager::update_column2term(const std::vector<std::shared_ptr<TableManager>> &tms,
                                         std::vector<BTreeIterator<VariantPage>> &btits, 
                                         std::vector<RecordManager> &rms,
                                         std::unordered_map<std::string, std::shared_ptr<TermExpr>> &column2term)
{
    column2term.clear();
    for (size_t counter = 0; counter < tms.size(); ++counter)
    {
        auto tm = tms[counter];
        auto rm = rms[counter];
        auto btit = btits[counter];
        // Initialize each record manager
        rm.Open(*btit, false);
        for (size_t i = 0; i < tm->number_of_column(); ++i)
        {
            rm.Seek(tm->column_offset(i));
            size_t col_length = tm->column_length(i);
            switch (tm->column_type(i))
            {
            case Col_Type::COL_TYPE_INT:
            {
                int val;
                rm.Read(&val, col_length);
                auto term = std::make_shared<TermExpr>(val);
                column2term.insert({tm->table_name() + "." + tm->column_name(i), term});
                break;
            }
            case Col_Type::COL_TYPE_DOUBLE:
            {
                double val;
                rm.Read(&val, col_length);
                auto term = std::make_shared<TermExpr>(val);
                column2term.insert({tm->table_name() + "." + tm->column_name(i), term});
                break;
            }
            case Col_Type::COL_TYPE_BOOL:
            {
                bool val;
                rm.Read(&val, col_length);
                auto term = std::make_shared<TermExpr>(val);
                column2term.insert({tm->table_name() + "." + tm->column_name(i), term});
                break;
            }
            case Col_Type::COL_TYPE_DATE:
            {
                Date val;
                rm.Read(&val, col_length);
                auto term = std::make_shared<TermExpr>(val);
                column2term.insert({tm->table_name() + "." + tm->column_name(i), term});
                break;
            }
            case Col_Type::COL_TYPE_CHAR:
            case Col_Type::COL_TYPE_VARCHAR:
            {
                char *val = new char[col_length + 1];
                val[col_length] = '\0';
                rm.Read(val, col_length);
                auto term = std::make_shared<TermExpr>(val);
                column2term.insert({tm->table_name() + "." + tm->column_name(i), term});
                delete[] val;
                break;
            }
            case Col_Type::COL_TYPE_NULL:
            {
                auto term = std::make_shared<TermExpr>();
                column2term.insert({tm->table_name() + "." + tm->column_name(i), term});
            }
            default:
                break;
            }
        }
    }
}

void DatabaseManager::iterate_one_table(const std::shared_ptr<TableManager> &tm,
                                        std::vector<ColumnRef> &columns, ExprNode *condition)
{
    // Check whether the columns is empty or have column doesn't exist
    if (columns.empty())
    {
        columns.reserve(tm->number_of_column() - 1);
        for (size_t i = 0; i < tm->number_of_column() - 1; ++i)  // No need to add __rowid__
        {
            columns.emplace_back(tm->column_name(i));
        }
    }
    else
    {
        std::unordered_set<std::string> col_s;
        for (size_t i = 0; i < tm->number_of_column() - 1; ++i)  // No need to validate __rowid__ exists
        {
            col_s.insert(tm->column_name(i));
        }
        for (auto it = columns.begin(); it != columns.end(); ++it)
        {
            if (col_s.find(it->all_name()) == col_s.end()) 
            { 
                std::cout << "Error: no column named \"" << *it << "\"!" << std::endl;
                return;
            }
        }
    }

    // Print header
    for (auto it = columns.begin(); it != columns.end(); ++it)
    {
        if (it != columns.begin()) { std::cout << "\t"; }
        std::cout << it->all_name();
    }
    std::cout << std::endl;

    // Construct btree iterator and record manager
    int row_id = 1;
    auto pos = tm->GetRowPosition(row_id);
    BTreeIterator<VariantPage> btit{tm->pg(), pos};
    RecordManager rm{tm->pg()};
            
    std::unordered_map<std::string, std::shared_ptr<TermExpr>> column2term;
    // Print rows
    // The cartesian product
    while (true)
    {
        // Update column2term;
        update_column2term(tm, btit, rm, column2term);
        // Check condition
        if (condition != nullptr)
        {
            Expression expression{condition, column2term};
            if (!expression.term_.bval_) 
            { 
                btit.next();
                // Loop over
                if (btit.IsEnd()) { break; }
                continue; 
            }
        }
        // Print
        for (size_t i = 0; i < columns.size(); ++i)
        {
            if (i != 0) { std::cout << "\t"; }
            std::cout << *column2term[columns[i].all_name()];
        }
        std::cout << std::endl;

        btit.next();
        // Loop over
        if (btit.IsEnd()) { break; }
    }
}

void DatabaseManager::iterate_many_table(const std::vector<std::shared_ptr<TableManager>> &tms,
                                         std::vector<ColumnRef> &columns, ExprNode *condition)
{
    // Construct btree iterator vector and record manager vector
    std::vector<BTreeIterator<VariantPage>> btits;
    std::vector<RecordManager> rms;
    int row_id = 1;
    for (auto it = tms.begin(); it != tms.end(); ++it)
    {
        auto tm = *it;
        auto pos = tm->GetRowPosition(row_id);
        btits.emplace_back(tm->pg(), pos);
        rms.emplace_back(tm->pg());
    }

    // Check whether the columns is empty or have column doesn't exist
    if (columns.empty())
    {
        for (auto &tm : tms)
        {
            for (size_t i = 0; i < tm->number_of_column() - 1; ++i)  // No need to add __rowid__
            {
                columns.emplace_back(tm->table_name() + "." + tm->column_name(i));
            }
        }
    }
    else
    {
        std::unordered_set<std::string> col_s;
        for (auto &tm : tms)
        {
            for (size_t i = 0; i < tm->number_of_column() - 1; ++i)  // No need to validate __rowid__ exists
            {
                col_s.insert(tm->table_name() + "." + tm->column_name(i));
            }
        }
        for (auto it = columns.begin(); it != columns.end(); ++it)
        {
            if (col_s.find(it->all_name()) == col_s.end()) 
            { 
                std::cout << "Error: no column named \"" << *it << "\"!" << std::endl;
                return;
            }
        }
    }

    // Print header
    for (auto it = columns.begin(); it != columns.end(); ++it)
    {
        if (it != columns.begin()) { std::cout << "\t"; }
        std::cout << it->all_name();
    }
    std::cout << std::endl;

    std::unordered_map<std::string, std::shared_ptr<TermExpr>> column2term;
    // Iterate btree iterator function
    auto iterate_btree_iterator = [&]() -> std::vector<BTreeIterator<VariantPage>>::reverse_iterator {
        auto it = btits.rbegin();
        for (; it != btits.rend(); ++it)
        {
            it->next();
            if (it->IsEnd())
            {
                it->Reset();
            }
            else
            {
                break;
            }
        }
        return it;
    };
    // Print rows
    // The cartesian product
    while (true)
    {
        // Update column2term;
        update_column2term(tms, btits, rms, column2term);
        // Check condition
        if (condition != nullptr)
        {
            Expression expression{condition, column2term};
            if (!expression.term_.bval_) 
            { 
                auto it = iterate_btree_iterator();
                // Loop over
                if (it == btits.rend()) { break; }
                continue; 
            }
        }
        // Print
        for (size_t i = 0; i < columns.size(); ++i)
        {
            if (i != 0) { std::cout << "\t"; }
            std::cout << *column2term[columns[i].all_name()];
        }
        std::cout << std::endl;

        auto it = iterate_btree_iterator();
        // Loop over
        if (it == btits.rend()) { break; }
    }
}

void DatabaseManager::iterate_one_table(const std::shared_ptr<TableManager> &tm, std::vector<ColumnRef> &columns, ExprNode *condition,
                                        std::vector<std::shared_ptr<TermExpr>> cols, std::vector<std::shared_ptr<TermExpr>> vals)
{
    // Check whether the columns is empty or have column doesn't exist
    if (columns.empty())
    {
        columns.reserve(tm->number_of_column() - 1);
        for (size_t i = 0; i < tm->number_of_column() - 1; ++i)  // No need to add __rowid__
        {
            columns.emplace_back(tm->column_name(i));
        }
    }
    else
    {
        std::unordered_set<std::string> col_s;
        for (size_t i = 0; i < tm->number_of_column() - 1; ++i)  // No need to validate __rowid__ exists
        {
            col_s.insert(tm->column_name(i));
        }
        for (auto it = columns.begin(); it != columns.end(); ++it)
        {
            if (col_s.find(it->all_name()) == col_s.end()) 
            { 
                std::cout << "Error: no column named \"" << *it << "\"!" << std::endl;
                return;
            }
        }
    }

    // Print header
    for (auto it = columns.begin(); it != columns.end(); ++it)
    {
        if (it != columns.begin()) { std::cout << "\t"; }
        std::cout << it->all_name();
    }
    std::cout << std::endl;

    // find row_id if where statement can use index
    auto get_index = [&](const std::string &name) -> size_t {
        for (size_t i = 0; i < tm->number_of_column(); ++i)
        {
            if (name == tm->column_name(i))
            {
                return i;
            }
        }
        return tm->number_of_column();
    };
    std::vector<std::vector<int>> rows_arr;
    for (size_t i = 0; i < cols.size(); ++i)
    {
        size_t index = get_index(cols[i]->ref_.column_name);
        if ((1 << index) & tm->table_header().flag_index)
        {
            // this col is an index
            unsigned int page_id = tm->indices(index)->root_page_id();
            auto index_btr = std::make_shared<IndexBTree>(tm->pg(), page_id, tm->column_length(index) + 5, 
                                                    IndexManager::GetIndexComparer(tm->column_type(index)));
            const char *key = nullptr;
            switch (tm->column_type(index))
            {
            case Col_Type::COL_TYPE_INT:
                key = reinterpret_cast<const char*>(&vals[i]->ival_);
                break;
            case Col_Type::COL_TYPE_DOUBLE:
                key = reinterpret_cast<const char*>(&vals[i]->dval_);
                break;
            case Col_Type::COL_TYPE_BOOL:
                key = reinterpret_cast<const char*>(&vals[i]->bval_);
                break;
            case Col_Type::COL_TYPE_DATE:
                key = reinterpret_cast<const char*>(&vals[i]->dval_);
                break;
            case Col_Type::COL_TYPE_CHAR:
            case Col_Type::COL_TYPE_VARCHAR:
                key = vals[i]->sval_.c_str();
                break;
            default:
                break;
            }
            auto rows = index_btr->find_rows(key);
            if (!rows.empty())
            {
                rows_arr.push_back(rows);
            }
        }
    }
    // Get intersection of rows
    auto get_intersection = [&](std::vector<int> &vec1, std::vector<int> &vec2) {
        std::unordered_set<int> s1(vec1.begin(), vec1.end());
        std::unordered_set<int> s2(vec2.begin(), vec2.end());
        std::vector<int> res;
        for (const auto &item : s1)
        {
            if (s2.find(item) != s2.end())
            {
                res.push_back(item);
            }
        }
        return res;
    };
    if (rows_arr.empty())
    {
        // no rows satisfy the where condition
        return;
    }
    auto arr = rows_arr[0];
    for (int i = 1; i < rows_arr.size(); ++i)
    {
        arr = get_intersection(arr, rows_arr[i]);
    }

    // Construct btree iterator and record manager
    BTreeIterator<VariantPage> btit{arr, tm->btr()};
    RecordManager rm{tm->pg()};

    std::unordered_map<std::string, std::shared_ptr<TermExpr>> column2term;
    // Print rows
    // The cartesian product
    while (true)
    {
        // Update column2term;
        update_column2term(tm, btit, rm, column2term);
        // Check condition
        if (condition != nullptr)
        {
            Expression expression{condition, column2term};
            if (!expression.term_.bval_) 
            { 
                btit.next();
                // Loop over
                if (btit.IsEnd()) { break; }
                continue; 
            }
        }
        // Print
        for (size_t i = 0; i < columns.size(); ++i)
        {
            if (i != 0) { std::cout << "\t"; }
            std::cout << *column2term[columns[i].all_name()];
        }
        std::cout << std::endl;

        btit.next();
        // Loop over
        if (btit.IsEnd()) { break; }
    }
}

void DatabaseManager::iterate_many_table(const std::vector<std::shared_ptr<TableManager>> &tms, std::vector<ColumnRef> &columns, ExprNode *condition,
                                         std::vector<std::shared_ptr<TermExpr>> cols, std::vector<std::shared_ptr<TermExpr>> vals)
{
    // Check whether the columns is empty or have column doesn't exist
    if (columns.empty())
    {
        for (auto &tm : tms)
        {
            for (size_t i = 0; i < tm->number_of_column() - 1; ++i)  // No need to add __rowid__
            {
                columns.emplace_back(tm->table_name() + "." + tm->column_name(i));
            }
        }
    }
    else
    {
        std::unordered_set<std::string> col_s;
        for (auto &tm : tms)
        {
            for (size_t i = 0; i < tm->number_of_column() - 1; ++i)  // No need to validate __rowid__ exists
            {
                col_s.insert(tm->table_name() + "." + tm->column_name(i));
            }
        }
        for (auto it = columns.begin(); it != columns.end(); ++it)
        {
            if (col_s.find(it->all_name()) == col_s.end()) 
            { 
                std::cout << "Error: no column named \"" << *it << "\"!" << std::endl;
                return;
            }
        }
    }

    // Print header
    for (auto it = columns.begin(); it != columns.end(); ++it)
    {
        if (it != columns.begin()) { std::cout << "\t"; }
        std::cout << it->all_name();
    }
    std::cout << std::endl;

    // Divide cols and vals by table
    std::vector<std::vector<std::shared_ptr<TermExpr>>> tcols;
    tcols.resize(tms.size());
    std::vector<std::vector<std::shared_ptr<TermExpr>>> tvals;
    tvals.resize(tms.size());
    for (size_t i = 0; i < cols.size(); ++i)
    {
        for (size_t j = 0; j < tms.size(); ++j)
        {
            if (cols[i]->ref_.table_name == tms[j]->table_name())
            {
                tcols[j].push_back(cols[i]);
                tvals[j].push_back(vals[i]);
            }
        }
    }

    // Construct btree iterator vector and record manager vector
    std::vector<BTreeIterator<VariantPage>> btits;
    std::vector<RecordManager> rms;

    // find row_id if where statement can use index
    for (int tmi = 0; tmi < tms.size(); ++tmi)
    {
        auto tm = tms[tmi];
        auto get_index = [&](const std::string &name) -> size_t {
            for (size_t i = 0; i < tm->number_of_column(); ++i)
            {
                if (name == tm->column_name(i))
                {
                    return i;
                }
            }
            return tm->number_of_column();
        };
        std::vector<std::vector<int>> rows_arr;
        for (size_t i = 0; i < tcols[tmi].size(); ++i)
        {
            size_t index = get_index(tcols[tmi][i]->ref_.column_name);
            if ((1 << index) & tm->table_header().flag_index)
            {
                // this col is an index
                unsigned int page_id = tm->indices(index)->root_page_id();
                auto index_btr = std::make_shared<IndexBTree>(tm->pg(), page_id, tm->column_length(index) + 5, 
                                                        IndexManager::GetIndexComparer(tm->column_type(index)));
                const char *key = nullptr;
                switch (tm->column_type(index))
                {
                case Col_Type::COL_TYPE_INT:
                    key = reinterpret_cast<const char*>(&vals[i]->ival_);
                    break;
                case Col_Type::COL_TYPE_DOUBLE:
                    key = reinterpret_cast<const char*>(&vals[i]->dval_);
                    break;
                case Col_Type::COL_TYPE_BOOL:
                    key = reinterpret_cast<const char*>(&vals[i]->bval_);
                    break;
                case Col_Type::COL_TYPE_DATE:
                    key = reinterpret_cast<const char*>(&vals[i]->dval_);
                    break;
                case Col_Type::COL_TYPE_CHAR:
                case Col_Type::COL_TYPE_VARCHAR:
                    key = vals[i]->sval_.c_str();
                    break;
                default:
                    break;
                }
                auto rows = index_btr->find_rows(key);
                if (!rows.empty())
                {
                    rows_arr.push_back(rows);
                }
            }
        }
        // Get intersection of rows
        auto get_intersection = [&](std::vector<int> &vec1, std::vector<int> &vec2) {
            std::unordered_set<int> s1(vec1.begin(), vec1.end());
            std::unordered_set<int> s2(vec2.begin(), vec2.end());
            std::vector<int> res;
            for (const auto &item : s1)
            {
                if (s2.find(item) != s2.end())
                {
                    res.push_back(item);
                }
            }
            return res;
        };
        if (rows_arr.empty())
        {
            // no rows satisfy the where condition
            continue;
        }
        auto arr = rows_arr[0];
        for (int i = 1; i < rows_arr.size(); ++i)
        {
            arr = get_intersection(arr, rows_arr[i]);
        }
        btits.emplace_back(arr, tm->btr());
        rms.emplace_back(tm->pg());
    }

    std::unordered_map<std::string, std::shared_ptr<TermExpr>> column2term;
    // Iterate btree iterator function
    auto iterate_btree_iterator = [&]() -> std::vector<BTreeIterator<VariantPage>>::reverse_iterator {
        auto it = btits.rbegin();
        for (; it != btits.rend(); ++it)
        {
            it->next();
            if (it->IsEnd())
            {
                it->Reset();
            }
            else
            {
                break;
            }
        }
        return it;
    };
    // Print rows
    // The cartesian product
    while (true)
    {
        // Update column2term;
        update_column2term(tms, btits, rms, column2term);
        // Check condition
        if (condition != nullptr)
        {
            Expression expression{condition, column2term};
            if (!expression.term_.bval_) 
            { 
                auto it = iterate_btree_iterator();
                // Loop over
                if (it == btits.rend()) { break; }
                continue; 
            }
        }
        // Print
        for (size_t i = 0; i < columns.size(); ++i)
        {
            if (i != 0) { std::cout << "\t"; }
            std::cout << *column2term[columns[i].all_name()];
        }
        std::cout << std::endl;

        auto it = iterate_btree_iterator();
        // Loop over
        if (it == btits.rend()) { break; }
    }
}