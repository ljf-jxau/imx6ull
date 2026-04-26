#ifndef DATABASE_H
#define DATABASE_h
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <string.h>
using namespace std;
class DataBase
{
private:
    MYSQL*mysql;
public:
    DataBase();
    ~DataBase();
    bool database_connect();
    void database_disconnect();
    bool database_init_table();
    bool database_user_exist(string appid);
    void database_add_user(string a,string p);
    bool database_password_correct(string,string);
    bool database_user_bind(string,string &);
    void database_bind_user(string,string);

};

#endif