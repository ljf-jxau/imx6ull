#include <iostream>
#include "server.h"
using namespace std;
Server::Server()
{
    //初始化事件集合
    m_base=event_base_new();
    //初始化数据库
    m_database=new DataBase;
    if(!m_database->database_init_table())
    {
       cout<<"数据库初始化失败"<<endl;
       exit(1);
    }

    //创建player对象，用于音箱与app的交互
    m_p=new Player();

}

void Server::timeout_cb(evutil_socket_t fd, short events, void *arg)
{
    Player*p=(Player*)arg;
    p->player_traverse_list();

}

void Server::server_start_timer()
{
    // 1. 定义定时器事件对象：struct event 是libevent中描述"事件"的核心结构体，这里专门用于定时器
    struct event tv;  

    // 2. 定义时间结构体：用于指定定时器的触发间隔（秒+微秒）
    struct timeval t;

    // 3. 初始化（赋值）定时器事件
    // event_assign函数作用：将定时器事件tv与事件循环m_base绑定，配置触发规则和回调
    // 参数说明：
    // &tv       - 要初始化的定时器事件对象
    // m_base    - 事件循环核心（event_base），定时器需挂载到事件循环中才能运行
    // -1        - 定时器无需关联文件描述符（fd=-1是libevent定时器的固定写法）
    // EV_PERSIST - 事件持久化标志：定时器触发后不会自动销毁，会按间隔重复触发（否则触发一次就失效）
    // timeout_cb - 定时器触发时要执行的回调函数（用户自定义的定时任务逻辑）
    // m_p       - 传递给回调函数timeout_cb的自定义参数（通常是当前类的指针/业务上下文）
    // 返回值：-1表示初始化失败，0表示成功
    if (event_assign(&tv, m_base, -1, EV_PERSIST, timeout_cb, m_p) == -1)
    {
        cout << "初始化定时器事件失败" << endl;
        return;  // 补充：初始化失败后建议退出，避免后续无效操作
    }

    // 4. 清空时间结构体的初始值（避免随机垃圾值影响定时精度）
    // evutil_timerclear是libevent的工具函数，将tv_sec（秒）和tv_usec（微秒）置为0
    evutil_timerclear(&t);
    
    // 5. 设置定时器触发间隔：2秒（微秒部分tv_usec未设置，默认0，即精确到秒）
    t.tv_sec = 2;  // 秒数：2秒
    // t.tv_usec = 0; // 微秒数：可选，这里省略（默认0）

    // 6. 将定时器事件注册到事件循环中
    // event_add作用：把配置好的事件（这里是定时器）添加到event_base的监听队列，等待触发
    event_add(&tv, &t);

    // 7. 启动事件循环（核心）
    // event_base_dispatch作用：进入无限循环，持续监听/处理已注册的事件（这里是定时器事件）
    // 注意：该函数会阻塞当前线程，直到所有事件被移除/退出循环（如调用event_base_loopbreak）
    event_base_dispatch(m_base);
}
void Server::listen(const char*ip,int port)
{
    struct sockaddr_in server_info;
    int len=sizeof(server_info);
    memset(&server_info,0,len);
    server_info.sin_family=AF_INET;
    server_info.sin_port=htons(port);
    server_info.sin_addr.s_addr=inet_addr(ip);
    //用于创建并绑定监听套接字的核心函数(端口可复用，socket不用手动释放)
    //evconnlistener_new_bind是 Libevent 库中用于创建并绑定监听器的核心函数。
    //它相当于一次性完成了 socket()、bind()、listen()和 accept()的初始化工作，并将监听套接字注册到事件循环中
    struct evconnlistener *listener=evconnlistener_new_bind
    (m_base,listener_cb,this,LEV_OPT_CLOSE_ON_FREE |LEV_OPT_REUSEABLE,5,(struct sockaddr*)&server_info,len);
    if(listener==NULL)
    {
        cout<<"bind error"<<endl;
        int err = EVUTIL_SOCKET_ERROR();
        cerr << "Error: " << evutil_socket_error_to_string(err) << endl;
    }

    //启动定时器并监听
    server_start_timer();
    // //监听集合
    // event_base_dispatch(m_base);        //死循环,集合为空时才会出来（listen这个事件从m_base里面删掉才会出来）
    
    //释放对象
    evconnlistener_free(listener);
    event_base_free(m_base);
}

struct event_base* Server::server_get_base()
{
    return m_base;

}

 //一旦有客户端发起连接请求，就会触发该函数
void Server::listener_cb(struct evconnlistener *l, evutil_socket_t fd,struct sockaddr *c, int socklen, void *arg)
{
    struct sockaddr_in*client_info=(struct sockaddr_in *)c;
    cout<<"[new client connection]";
    cout<<inet_ntoa(client_info->sin_addr)<<":";
    cout<<client_info->sin_port<<endl;
    Server*s=(Server*)arg;
    struct event_base *base=s->server_get_base();

    //struct event_base *base=(struct event_base *)arg;
    //BEV_OPT_CLOSE_ON_FREE释放 bufferevent 时自动 close(fd)，无需手动关闭 FD
    struct bufferevent*bev=bufferevent_socket_new(base,fd,BEV_OPT_CLOSE_ON_FREE);
    if(NULL==bev)
    {
        cout<<"bufferevent_socket_new error"<<endl;
    }
    //是 libevent 中为 bufferevent（缓冲事件对象）绑定回调函数 的核心函数
    bufferevent_setcb(bev,read_cb,NULL,event_cb,s);
    bufferevent_enable(bev,EV_READ);//使能某个事件为可读
}

void Server::server_read_date(struct bufferevent*bev,char*msg)
{
    char buf[256]={0};
    size_t size=0;
    while(1)
    {
        size+=bufferevent_read(bev,buf+size,4-size);
        if(size>=4) break;     
    }
    int len=*(int*)buf;
    memset(buf,0,4);
    size=0;
    while(1)
    {
        size+=bufferevent_read(bev,buf+size,len-size);
        if(size>=len) break;     
    }
    memcpy(msg,buf,strlen(buf));
    cout<<"长度："<<len<<"消息："<<msg<<endl;  
}

void Server::server_send_date(struct bufferevent *bev,Json::Value &v)
{
    char msg[1024]={0};
    //把 JSON 数据对象（Json::Value类型）转换为字符串类型
    string Sendstr=Json::FastWriter().write(v);
    int len=Sendstr.size();
    memcpy(msg,&len,sizeof(int));
    memcpy(msg+sizeof(int),Sendstr.c_str(),len);
    if(bufferevent_write(bev,msg,len+sizeof(int))==-1)
    {
       cout<<"bufferevent_write 出错"<<endl;
    }

}

void Server::server_get_music(struct bufferevent *bev,string s)
{
    Json::Value val;
    Json::Value arr;
    list<string>l;//定义一个双向链表
    char path[128]={0};
    sprintf(path,"/home/zd/music/%s",s.c_str());//歌手名,string转换为const char*类型
    DIR*dir=opendir(path);//用于打开一个目录
    if(NULL==dir)
    {
        perror("opendir");
        return;
    }
    struct dirent*d;
    while((d=readdir(dir))!=NULL)//逐个读取目录中的文件和子目录信息
    {
        if(d->d_type!=DT_REG)  //如果不是普通文件
          continue;
        if(!(strstr(d->d_name,".mp3")))
          continue;
        char name[128]={0};
        sprintf(name,"%s/%s",s.c_str(),d->d_name);
        l.push_back(d->d_name);  
    }
    //随机选择5首歌曲
    list<string>::iterator it=l.begin();
    srand(time(NULL));
    int count=rand()%(l.size()-4);
    for(int i=0;i<count;i++)
    {
        it++;
    }
    for(int i=0;i<5 &&it!=l.end();i++,it++)
    {
        arr.append(*it);
    }
    val["cmd"]="reply_music";
    val["music"]= arr;

    server_send_date(bev,val);

}

void Server::server_player_handler(struct bufferevent *bev,Json::Value &v)
{
    if(v["cmd"]=="info")//音箱每2秒上传一次信息到服务器，如果app在线的话就把这些信息直接转发给app
    {
        m_p->player_update_list(bev,v,this);
    }
    else if(v["cmd"]=="app_info")//app每两秒上传一次信息给服务器，就更新一下信息，不做其它事情
    {
        m_p->player_app_update_list(bev,v);
    }
    else if(v["cmd"]=="upload_music")//如果app在线的话将音箱的5首歌曲转发到app(为了更新app)
    {
        m_p->player_upload_music(this,bev,v);
    }
    else if(v["cmd"]=="app_get_music")//app要获取音乐
    {
       m_p->player_get_music(this,bev,v);
    }
    
    //将app的命令转发给音箱，音箱不在线再返回给app
    else if(v["cmd"]=="app_start"||v["cmd"]=="app_stop"||v["cmd"]=="app_suspend"||v["cmd"]=="app_continue"||
    v["cmd"]=="app_prior"||v["cmd"]=="app_next"||v["cmd"]=="app_voice_up"||
    v["cmd"]=="app_voice_down"||v["cmd"]=="app_circle"||v["cmd"]=="app_sequence")
    {
        m_p->player_option(this,bev,v);
    }
    //将发送给音箱的命令转发回app
    else if(v["cmd"]=="app_start_reply"||v["cmd"]=="app_stop_reply"||v["cmd"]=="app_suspend_reply"||v["cmd"]=="app_continue_reply"||
    v["cmd"]=="app_prior_reply"||v["cmd"]=="app_next_reply"||v["cmd"]=="app_voice_up_reply"||
    v["cmd"]=="app_voice_down_reply"||v["cmd"]=="app_circle_reply"||v["cmd"]=="app_sequence_reply")
    {
        m_p->player_reply_option(this,bev,v);

    }
}

void Server::server_app_register(struct bufferevent *bev,Json::Value &v)
{ 
    Json::Value val;
    //打开数据库
    m_database->database_connect();

    string appid=v["appid"].asString();
    string password=v["password"].asString();

    //判断用户是否存在
    if(m_database->database_user_exist(appid))//用户存在
    {
        val["result"]="failure";
        cout<<"用户存在，注册失败"<<endl;
    }
    //如果不存在，修改数据库
    else
    {
        m_database->database_add_user(appid,password);
         val["result"]="success";
         cout<<"用户注册成功"<<endl;
    }
    //断开数据库
    m_database->database_disconnect();

    //回复
    val["cmd"]="app_register_reply";
    server_send_date(bev,val);

}

void Server::server_app_login(struct bufferevent *bev,Json::Value &v)
{
    //连接数据库
    Json::Value val;
    m_database->database_connect();

    string appid=v["appid"].asString();
    string password=v["password"].asString();
    string deviceid;

    //判断用户是否存在
    do
    {
        if(!m_database->database_user_exist(appid))//用户不存在
        {
        val["result"]="not_exist";
        cout<<"用户不存在，注册失败"<<endl;
        break;
        }
        //判断密码是否正确
        if(!m_database->database_password_correct(appid,password))
        {
            val["result"]="password_error";
            break;
        }
        //是否绑定
        if(m_database->database_user_bind(appid,deviceid))
        {
            val["result"]="bind";
            val["deviceid"]=deviceid;
        }
        else
        {
            val["result"]="not_bind";
        }
    }while(0);
    //断开数据库
    m_database->database_disconnect();

    //返回app
    val["cmd"]="app_login_reply";
    server_send_date(bev,val);

}

void Server::server_app_bind(struct bufferevent *bev,Json::Value &v)
{
    string appid=v["appid"].asString();
    string deviceid=v["deviceid"].asString();
    //连接数据库
    m_database->database_connect();

    //修改数据库
    m_database->database_bind_user(appid,deviceid);

    //关闭数据库
    m_database->database_disconnect();

    //返回
    Json::Value val;
    val["cmd"]="app_bind_reply";
    val["result"]="success";
    server_send_date(bev,val);
}

void Server::server_app_offline(struct bufferevent *bev)
{
    m_p->player_app_offline(bev);
}

//有客户端发数据过来时会触发该回调函数
void Server::read_cb(struct bufferevent*bev,void*ctx)
{
    char buf[1024]={0};
    Server*s=(Server*)ctx;
    s->server_read_date(bev,buf);
    //解析json
    Json::Reader reader;     //用于解析的对象
    Json::Value value;        //存放解析的结果

    if(!reader.parse(buf,value))//jsoncpp 库中用于解析 JSON 字符串的核心函数
    {
        cout<<"json 出错"<<endl;
        return;
    }
    if(value["cmd"]=="get_music_list") //音箱获取获取音乐数据，服务器返回5首歌曲回去
    {
        s->server_get_music(bev,value["singer"].asString());
    }
    else if(value["cmd"]=="app_register")//app端进行一个注册，注册成功还是失败都会返回给app
    {
        s->server_app_register(bev,value);
    }
    else if(value["cmd"]=="app_login")//登入
    {
        s->server_app_login(bev,value);
    }
    else if(value["cmd"]=="app_bind")//app的绑定音箱
    {
        s->server_app_bind(bev,value);
    }
    else if(value["cmd"]=="app_offline")//app下线
    {
        s->server_app_offline(bev);
    }
    else
    {
        s->server_player_handler(bev,value);
    }
}

void Server::server_client_offline(struct bufferevent *bev)
{
    m_p->player_offline(bev);


}

void Server::event_cb(struct bufferevent *bev, short what, void *ctx)
{
    Server*ser=(Server*)ctx;
    if(what & BEV_EVENT_EOF)
    {
        ser->server_client_offline(bev);//app或者音箱异常下线处理
    }

}

Server::~Server()
{
    if(m_database)
    {
        delete m_database;
    }
    if(m_p)
    {
        delete m_p;
    }
}