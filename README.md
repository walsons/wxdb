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
    id          int            PRIMARY KEY AUTO_INCREMENT,
    name        char(32)       NOT NULL ,
    email       VARCHAR(255),  UNIQUE
    age         int,
    height      DOUBLE,
    Country     Char(32)       DEFAULT "China",
    CHECK (age>=18)
);
```

keywords is not case sensitive.
