#ifndef PLAYER_H
#define PLAYER_H
#include <stdio.h>
#include <time.h>
#include <list>
#include <string>
#include <event2/event.h>
#include <jsoncpp/json/json.h>
class Server;
using namespace std;
struct PlayerInfo
{
    string deviceid;//音箱号
    string appid;//app号
    string cur_music;//音乐名字
  
    int valume;//音量
    int mode;//播放模式
    time_t d_time; //记录音箱上报的时间
    time_t a_time; //记录app上报的时间

    struct bufferevent*d_bev;    //对应音箱事件
    struct bufferevent*a_bev;     //对应app事件

};

class Player
{
private:
   list<PlayerInfo>*info;

public:
    Player();
    void player_update_list(struct bufferevent *bev,Json::Value &v,Server*ser);//更新设备链表里面的信息，并将信息发送给app
    void player_app_update_list(struct bufferevent *bev,Json::Value &v);//app每两秒上传一次信息给服务器
    void player_traverse_list();//用来确定音箱和app有没有离线
    void player_upload_music(Server*ser,struct bufferevent *bev,Json::Value &v);//将音箱的5首歌曲转发到app
    void player_option(Server*ser,struct bufferevent *bev,Json::Value &v);////将指令转发给设备
    void player_reply_option(Server*ser,struct bufferevent *bev,Json::Value &v);//将指令回复给app
    void player_offline(struct bufferevent *bev);//app或者音箱异常下线处理
    void player_app_offline(struct bufferevent *bev);//app下线
    void player_get_music(Server*ser,struct bufferevent *bev,Json::Value &v);//音箱获取歌曲
    ~Player();
};

#endif