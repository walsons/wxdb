# wxdb

This is a simple relational database aims to implement this sql statement:

```sql
CREATE DATABASE database_name;
# example
CREATE DATABASE mydb;
```

```sql
DROP DATABASE database_name;
# example
DROP DATABASE mydb;
```

```sql
USE database_name;
# example
USE mydb;
```

```sql
CREATE TABLE table_name (
    column1 datatype constraint,
    column2 datatype constraint,
    column3 datatype constraint,
    constraint,
    ....
);
# example
CREATE TABLE customers (
    id          int            
    name        char(32)       NOT NULL ,
    email       VARCHAR(255),  UNIQUE
    age         int,
    height      DOUBLE,
    country     Char(32)       DEFAULT "China",
    sign_up     Date,
    PRIMARY KET (id),
    CHECK(age>=18 AND age<= 60)
);
```

keywords is not case sensitive.
