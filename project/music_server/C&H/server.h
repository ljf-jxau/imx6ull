#ifndef SERVER_H
#define SERVER_H
//#include <event2/event.h>
#include <event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <arpa/inet.h>   // 核心：inet_addr() 定义
#include <netinet/in.h>  // 辅助：in_addr_t、struct sockaddr_in 定义
#include <sys/socket.h>  // 辅助：socket 相关类型
#include <string.h>
#include <stdlib.h>
#include <list>
#include <time.h>
#include <jsoncpp/json/json.h>
#include <sys/types.h>  // 提供 DIR、ino_t 等类型定义
#include <dirent.h>     // 核心头文件（opendir/closedir/readdir 均在此）
#include <stdio.h>
#include "database.h"
#include "player.h"
#define IP "192.168.10.100"
#define PORT 8001

#define SEQUENCE 1
#define CIRCLE 2
using namespace std;
class Server
{
private:
    struct event_base *m_base;      //事件集合
    DataBase*m_database;          //数据库对象
    Player*m_p;          //链表类对象
public:
    Server();
    void listen(const char*ip,int port);//服务器的绑定函数
    struct event_base*server_get_base();//返回事件结合函数
    //当有客户端连接的回调函数
    static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,struct sockaddr *addr, int socklen, void *ptr);
    void server_read_date(struct bufferevent*bev,char*msg);//服务器读取数据
    static void read_cb(struct bufferevent*bev,void*ctx);//当客户端有数据可读时的回调函数
    static void event_cb(struct bufferevent *bev, short what, void *ctx);//如连接断开、超时、错误）时触发。
    void server_get_music(struct bufferevent *bev,string s);//音箱端获取音乐
    void server_send_date(struct bufferevent *bev,Json::Value &v);//服务端发送数据函数
    void server_player_handler(struct bufferevent *bev,Json::Value &v);//接受app或者音箱的一些请求信息
    void server_start_timer();//定时器函数
    void server_app_register(struct bufferevent *bev,Json::Value &v);//app端的注册
    void server_app_login(struct bufferevent *bev,Json::Value &v);//app端的登录
    void server_app_bind(struct bufferevent *bev,Json::Value &v);//app绑定音箱
    void server_client_offline(struct bufferevent *bev);//app或者音箱异常下线处理
    void server_app_offline(struct bufferevent *bev);//app下线
    static void timeout_cb(evutil_socket_t fd, short events, void *arg);//定时器超时回调函数
    ~Server();

};
#endif