# ChatTogetherServer
聊天室服务器

## 配置
1. 安装mysql开发包：`sudo yum install mysql-devel`
2. 安装jsoncpp开发包：`sudo yum install jsoncpp-devel -y`
3. 创建数据表
```mysql
create database chat_togethor charset=utf8mb4;

use chat_togethor;

create table user(
    userid int primary key auto_increment,	
    username varchar(32) comment '昵称',
    password varchar(32),
    state tinyint comment '用户在线状态'
) charset=utf8mb4;

create table chat_history(
    msgid int primary key auto_increment,
    msgtime varchar(32),
    msgsrc int comment '用户id',
    msg varchar(1024)
) charset=utf8mb4;
```
4. 数据库用户：root，密码：123456

编译时参数：
```
-lpthread -ljsoncpp `mysql_config --cflags --libs`
```

编译：
在src目录下`make`命令

运行：
在out目录下`./main`
