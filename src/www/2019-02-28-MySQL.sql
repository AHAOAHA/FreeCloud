-- 2019-02-28
/* 复习

数据库一套管理数据的软件

MySQL（MariaDB）	Oracle		SqlServer	Sqlite

client  ---> Service -- Database(DB) -- Table
客户端		 服务		库				表

数据存储引擎（数据以不同的形式管理在文件系统上）
MyISAM vs InnoDB

1. 库
建库 ***
	CREATE DATABASE `hello`
	（GBK windows主要使用，一个中文两个字节/UTF-8 MySQL 一个字符三个字节）
删库 / 修改库 *
	DROP DATABLE `hello`
查看库信息
	SHOW DATABASES;
	SHOW CREATE DATABASE `hello`
	
2. 表

	1）支持三种数据类型
		1. 数值类型
			INT (TINY/SMALL/BIG)
			FLOAT/DOUBLE
			DECIMAL
		2. 字符串|二进制字节流
			CHAR
			VARCHAR
			TEXT
			BLOB
		3. 日期+时间
			DATETIME
			TIMESTAMP
			DATE
			TIME
			
	2）约束
		1. NOT NULL
		2. DEFAULT
		3. COMMENT 注释
		4. ZEROFILL （括号在不同类型上起的作用）
		5. 主键		*****	保证每个表里一定有一个主键
		6. 自增 ID	*****	最好的情况，直接用自增ID做主键
		7. 唯一键
		
		主键 vs 唯一键
		相同点：
			不允许重复
		不同点：
		一张表里只能有一个主键（允许复合主键）
		一张表里允许有多个唯一键
		
		主键 NOT NULL
		唯一键允许 NULL
		
		8. 外键（InnoDB 支持，MyISAM 不支持）
		用于外键的字段，要保证两张表类型一致
		
	语句：
		建表 ***
		CREATE TABLE `students` (
			`id` INT PRIMARY KEY AUTO_INCREMENT,
			`name` VARCHAR(20) NOT NULL,
			`gender` TINYINT NOT NULL DEFAULT 0 
					COMMENT '0: 保密，1:  女, 2: 男',
			`registered_at` DATETIME NOT NULL
		);
		
		查看表信息 ***
		SHOW TABLES;
		DESC `students`;
		SHOW CREATE TABLE `students`;
		
		删表 *
		DROP TABLE `students`
		
		修改表 *
		ALTER TABLE `students` ...
		RENAME TABLE `old` TO `new`; **
		
3. 备份和恢复
	备份:
	在 Linux shell 下执行 mysqldump，命令行参数自己 man
	恢复:
	在 Linux shell 下执行 mysql < 备份的数据.sql
	在 MySQL shell 执行 source（保证 MySQL Server 和 source 文件在一个机器上）
*/


-- 商品订单练习
CREATE TABLE goods (
	id INT PRIMARY KEY AUTO_INCREMENT COMMENT '商品 ID',
	name VARCHAR(20) NOT NULL COMMENT '商品名称',
	price DECIMAL(10, 3) NOT NULL COMMENT '单价，单位元',
	category VARCHAR(10) NOT NULL COMMENT '类别',
	provider VARCHAR(10) NOT NULL DEFAULT '' COMMENT '供应商',
	joined_at DATETIME NOT NULL COMMENT '入库日期',
	created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '记录保存时间戳'
);

CREATE TABLE customers (
	id INT PRIMARY KEY AUTO_INCREMENT COMMENT '客户 ID',
	name VARCHAR(20) NOT NULL COMMENT '客户姓名',
	address VARCHAR(100) NOT NULL DEFAULT '' COMMENT '地址',
	email VARCHAR(100) NOT NULL DEFAULT '' COMMENT '电子邮箱',
	sex TINYINT NOT NULL DEFAULT 0 COMMENT '0: 保密，1: 女, 2: 男',
	idcard CHAR(18) NOT NULL DEFAULT '' COMMENT '身份证号',
	created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '记录保存时间戳'
);

CREATE TABLE purchase (
	id INT PRIMARY KEY AUTO_INCREMENT COMMENT '订单 ID',
	goods_id INT NOT NULL COMMENT '商品 ID',
	customer_id INT NOT NULL COMMENT '客户 ID',
	nums INT NOT NULL COMMENT '购买数量',
	bought_at DATETIME NOT NULL COMMENT '购买时间',
	created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '记录保存时间戳',
	FOREIGN KEY (goods_id) REFERENCES goods(id),
	FOREIGN KEY (customer_id) REFERENCES customers(id)
);

INSERT INTO goods (
	name, price, category, joined_at
) VALUES ('澳洲牛排', 2.00, '牛肉', '2018-02-28 19:00:00');

INSERT INTO customers (
	name
) VALUES ('林黛玉'), ('贾宝玉');

INSERT INTO purchase (
	goods_id, customer_id, nums, bought_at
) VALUES (1, 1, 10, '2018-12-25 00:00:00'), (1, 2, 10, '2019-01-01 00:00:00');

SELECT * FROM purchase, goods, customers 
WHERE 
	purchase.goods_id = goods.id 
	AND purchase.customer_id = customers.id\G