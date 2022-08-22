# wxdb

This is a simple relational database aims to implement these SQL statements(keywords are not case sensitive):

```sql
CREATE DATABASE database_name;

# example
CREATE DATABASE mydb;
```

```sql
USE database_name;

# example
USE mydb;
```

Support constraint type: **not null**, **unique**, **primary key**, **check** 

these constraint must write in a single line except **not null**.

```sql
CREATE TABLE table_name (
    column1 datatype constraint,
    column2 datatype constraint,
    column3 datatype constraint,
    constraint,
    ....
);

# example
CREATE TABLE users (
    id          INT,            
    name        CHAR(32)       NOT NULL,
    email       VARCHAR(255),
    age         INT,
    height      DOUBLE,
    country     CHAR(32)       DEFAULT "China",
    sign_up     DATE,
    UNIQUE (email),
    PRIMARY KEY (id),
    CHECK(age>=18 AND age<= 60)
);

CREATE TABLE comments (
    id          INT,
    user_id     INT        NOT NULL, 
    time        DATE       NOT NULL,
    contents    VARCHAR(255),
    PRIMARY KEY (id),
);
```

```sql
INSERT INTO table_name (column1, column2, column3, ...)
VALUES (value1, value2, value3, ...);

#example
INSERT INTO users (id, name, email, age, height, country, sign_up)
VALUES (1, "Walson", "walsons@163.com", 18, 180, "China", "2020-01-03");

INSERT INTO table_name
VALUES (value1, value2, value3, ...);

#example
INSERT INTO users
VALUES (1, "Walson", "walsons@163.com", 18, 180, "China", "2020-01-03");
```

```sql
SELECT *
FROM table_name
WHERE expression;

#example
SELECT * FROM users WHERE id < 10;

SELECT column1, column2, ...
FROM table_name
WHERE expression;

#example
SELECT id, name, sign_up FROM users WHERE id < 10;

# Select from multiple tables need to add table name before column name
SELECT *
FROM table_name, ...
WHERE expression;

#example
SELECT * FROM users, comments WHERE users.id = comments.user_id;

SELECT table_name.column1, table_name.column2, ...
FROM table_name, ...
WHERE expression;

#example
SELECT comments.id, users.name, comments.time, comments.contents
FROM users, comments
WHERE users.id = comments.user_id;
```

```sql
DELETE FROM table_name 
WHERE expresssion

#example
DELETE FROM users WHERE id < 10;
```

## Unit Test
Because of the static local object, the unit test should be run individually by SECTION.