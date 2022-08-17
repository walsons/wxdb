CREATE DATABASE mydb;

---

USE mydb;

---

CREATE TABLE users ( id INT, name CHAR(32) NOT NULL, email VARCHAR(255), age INT, height DOUBLE, country CHAR(32) DEFAULT "China", sign_up DATE, UNIQUE (email), PRIMARY KEY (id), CHECK(age>=18 AND age<= 60));

CREATE TABLE comments ( id INT, user_id INT NOT NULL, time DATE NOT NULL, contents VARCHAR(255), PRIMARY KEY (id), FOREIGN KEY (user_id) REFERENCES users(id));

---

INSERT INTO users (id, name, email, age, height, country, sign_up) VALUES (1, "Walson", "[walsons@163.com](mailto:walsons@163.com)", 18, 180, "China", "2020-01-03");

INSERT INTO users VALUES (1, "Walson", "[walsons@163.com](mailto:walsons@163.com)", 18, 180, "China", "2020-01-03");

---

SELECT * FROM table_name WHERE expression;

SELECT id, name, sign_up FROM users WHERE id < 10;