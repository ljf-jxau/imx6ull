#include "player.h"
#include "server.h"
Player::Player()
{
    info=new list<PlayerInfo>();
}

void Player::player_update_list(struct bufferevent *bev,Json::Value &v,Server*ser)
{
    //遍历链表，如果设备存在，更行链表并转发给app
    auto it=info->begin();
    for(;it!=info->end();it++)
    {
        if(it->deviceid==v["deviceid"].asString())
        {
            it->cur_music=v["cur_music"].asString();
            it->valume=v["volume"].asInt();
            it->mode=v["mode"].asInt();
            it->d_time=time(NULL);

            if(it->a_bev)
            {
                cout<<"到啦"<<endl;
                ser->server_send_date(it->a_bev,v);
            }
            return;

        }
    }
    //如果设备不存在，新建节点
    PlayerInfo p;
    p.deviceid=v["deviceid"].asString();
    p.cur_music=v["cur_music"].asString();
    p.valume=v["volume"].asInt();
    p.mode=v["mode"].asInt();
    p.d_time=time(NULL);
    p.d_bev=bev;
    p.a_bev=NULL;
    info->push_back(p);

    cout<<"第一次上报数据，已经新建立节点"<<endl;

}

void Player::player_app_update_list(struct bufferevent *bev,Json::Value &v)
{
    for(auto it=info->begin();it!=info->end();it++)
    {
        if(it->deviceid==v["deviceid"].asString())
        {
            it->a_time=time(NULL);
            it->appid=v["appid"].asString();
            it->a_bev=bev;
        }
    }
}

void Player::player_traverse_list()//用来确定音箱和app有没有离线
{
    cout<<"定时器事件遍历链表"<<endl;
    for(auto it=info->begin();it!=info->end();it++)
    {
        if(time(NULL)-it->d_time >6) //超过3次音箱离线
        {
            info->erase(it);
        }
        if(it->a_bev)
        {
            if(time(NULL)-it->a_time>6)//超过3次app离线
            {
                it->a_bev=NULL;
            }
        }
    }
}


void Player::player_upload_music(Server*ser,struct bufferevent *bev,Json::Value &v)
{
    for(auto it=info->begin();it!=info->end();it++)
    {
        if(it->d_bev==bev)
        {
            if(it->a_bev!=NULL)
            {
                ser->server_send_date(it->a_bev,v);
                cout<<"APP  在线，歌曲名字转发成功"<<endl;
            }
            break;
        }
        else
        {
            cout<<"APP不在线，歌曲转发失败"<<endl;
        }
        
    }

}

void Player::player_option(Server*ser,struct bufferevent *bev,Json::Value &v)//将指令转发给设备
{
    for(auto it=info->begin();it!=info->end();it++)
    {
        if(it->a_bev==bev)
        {
            if(it->d_bev)
            {
            ser->server_send_date(it->d_bev,v);
            cout<<"音箱在线，指令转发成功"<<endl;
            return;
            }
        }
    }
    //音箱不在线
    Json::Value value;
    string cmd=v["cmd"].asString();
    cmd+="_reply";
    value["cmd"]=cmd;
    value["result"]="offline";
    ser->server_send_date(bev,value);
    cout<<"音箱不在线，转发失败"<<endl;
}

void Player::player_reply_option(Server*ser,struct bufferevent *bev,Json::Value &v)//将指令回复给app
{
     for(auto it=info->begin();it!=info->end();it++)
    {
        if(it->d_bev==bev)
        {
            if(it->a_bev)
            {
            ser->server_send_date(it->a_bev,v);
            cout<<"app，指令转发成功"<<endl;
            break;
            }
        }
    }
}

void Player::player_offline(struct bufferevent *bev)//app或者音箱异常下线处理
{
    for(auto it=info->begin();it!=info->end();it++)
    {
        if(it->d_bev==bev)
        {
            cout<<"音箱异常下线处理"<<endl;
            info->erase(it);
            break;
        }
        if(it->a_bev==bev)
        {
            cout<<"APP异常下线处理"<<endl;
            it->a_bev=NULL;
            break;
        }
    }
}

void Player::player_app_offline(struct bufferevent *bev)
{
     for(auto it=info->begin();it!=info->end();it++)
     {
         if(it->a_bev==bev)
         {
            cout<<"APP正常下线处理"<<endl;
            it->a_bev=NULL;
            break;
         }
     }
}

void Player::player_get_music(Server*ser,struct bufferevent *bev,Json::Value &v)//音箱获取歌曲
{
    for(auto it=info->begin();it!=info->end();it++)
    {
        if(it->a_bev==bev)
        {
            ser->server_send_date(it->d_bev,v);
        }
    }
}

Player::~Player()
{
    if(info!=NULL)
    {
        delete info;
    }

}