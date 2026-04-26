#include "database.h"
using namespace std;
DataBase::DataBase()
{

}

bool DataBase::database_connect()//打开数据库
{
    mysql=mysql_init(NULL);//创建并初始化一个 MYSQL 结构体

    mysql=mysql_real_connect(mysql,"localhost","root","1","musicplayer",0,NULL,0);//建立与 MySQL 服务器的实际网络连接
    if(mysql==NULL)
    {
        cout<<"[DATABASE CONNECT FA]";
        cout<<mysql_error(mysql)<<endl;
        return false;
    }

    // if(mysql_query(mysql,"set names utf8;")!=0)
    // {

    // }
    return true;

}

void DataBase::database_disconnect()//关闭数据库
{
    mysql_close(mysql);

}

bool DataBase::database_init_table()//创建一张表
{
    if(!this->database_connect())
    return false;
    const char*sql="create table if not exists account(appid char(11),password varchar(16),deviceid varchar(8))charset utf8;";
    if(mysql_query(mysql,sql)!=0)//执行 SQL 语句
    {
        cout<<"[mysql query error]";
        cout<<mysql_errno(mysql)<<endl;
        return false;
    }
    this->database_disconnect();
    return true;

}

bool DataBase::database_user_exist(string appid)//查询该表有没有该appid
{
    char sql[256]={0};
    sprintf(sql,"select* from account where appid='%s';",appid.c_str());
    if(mysql_query(mysql,sql)!=0)
    {
        cout<<"[mysql query error]";
        cout<<mysql_errno(mysql)<<endl;
        return true;
    }

    MYSQL_RES*res=mysql_store_result(mysql);//从服务器获取查询结果，并将其存储在客户端内存中，方便后续处理。
    if(NULL==res)
    {
        cout<<"[mysql_store_result_error]";
        cout<<mysql_errno(mysql)<<endl;
        return true;
    }
    MYSQL_ROW row=mysql_fetch_row(res);//查阅查询结果(有数据的话返回非NULL，没有数据返回NULL)
    if(row==NULL)
    {
        return false;
    }
    else
    {
        return true;
    }
    
}

void DataBase::database_add_user(string a,string p)//插入新的appid和密码
{
    char sql[256]={0};
    sprintf(sql,"insert into account (appid,password) values('%s','%s');",a.c_str(),p.c_str());
    if(mysql_query(mysql,sql)!=0)
    {
        cout<<"[mysql query error]";
        cout<<mysql_errno(mysql)<<endl;
    }
}

bool DataBase::database_password_correct(string a,string p)//通过 appid 查找对应的密码，与用户提供的密码进行比较。
{
    char sql[256]={0};
    sprintf(sql,"select password from account where appid='%s';",a.c_str());
     if(mysql_query(mysql,sql)!=0)
    {
        cout<<"[mysql query error]";
        cout<<mysql_errno(mysql)<<endl;
        return false;
    }

    MYSQL_RES*res=mysql_store_result(mysql);
    if(NULL==res)
    {
        cout<<"[mysql_store_result_error]";
        cout<<mysql_errno(mysql)<<endl;
        return false;
    }
    MYSQL_ROW row=mysql_fetch_row(res);
    if(row==NULL)
    {
        return false;
    }
    if(strcmp(p.c_str(),row[0])==0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool DataBase::database_user_bind(string a,string &d)//查找对应的设备id用于绑定
{
    char sql[256]={0};
    sprintf(sql,"select deviceid from account where appid='%s';",a.c_str());
     if(mysql_query(mysql,sql)!=0)
    {
        cout<<"[mysql query error]";
        cout<<mysql_errno(mysql)<<endl;
        return false;
    }

    MYSQL_RES*res=mysql_store_result(mysql);
    if(NULL==res)
    {
        cout<<"[mysql_store_result_error]";
        cout<<mysql_errno(mysql)<<endl;
        return false;
    }
    MYSQL_ROW row=mysql_fetch_row(res);
    if(row==NULL)
    {
        return false;
    }
    if(NULL==row[0])
    {
        return false;
    }
    else
    {
        d=string(row[0]);
        return true;
    }
}

void DataBase::database_bind_user(string a,string d)//给appid绑定设备id
{
    char sql[256]={0};
    sprintf(sql,"update account set deviceid='%s' where appid='%s';",d.c_str(),a.c_str());
    if(mysql_query(mysql,sql)!=0)
    {
        cout<<"[mysql query error]";
        cout<<mysql_errno(mysql)<<endl;
        return ;
    }
}

DataBase::~DataBase()
{

}




