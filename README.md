# wxdb

This is a simple relational database aims to implement this sql statement(keywords is not case sensitive):

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

Support constraint type: **not null**, **unique**, **primary key**, **foreigh key**, **check** 

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
    contents    VARCHAR,
    PRIMARY KEY (id),
    FOREIGN KEY (user_id) REFERENCES users (id)
);
```

