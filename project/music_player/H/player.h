#ifndef SHM_H
#define SHM_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <ctype.h>
#include "link.h"
// 引入ALSA音频系统库，提供音量控制相关函数
//#include <alsa/asoundlib.h>
#include "socket.h"

//定义播放模式
#define SEQUENCE 1 //顺序播放
#define CIRCLE 2   //单曲循环
#define MUSICPATH   "http://192.168.10.100:8000/"
#define SHMKEY 1234
#define SHMSIZE 4096

//共享内存放的数据
typedef struct shm
{
    pid_t child_pid;        //子进程pid
    pid_t grand_pid;        //孙进程pid
    char cur_music[128];     //当前播放歌曲的名字
    int mode;               //播放模式

}Shm;

int init_shm();
void get_shm(Shm *s);
void set_shm(Shm*s);
int get_volume();//获取音量大小
void get_music(const char*s);
void get_singer(char*s);
int start_play();
void stop_play();
void suspend_play();
void continue_play();
void prior_play();
void next_play();
void set_volume(int volume);//设置音量大小为volume值
void play_music( char *name);
int m_mp3_end(const char *name);
void volume_up();//增加音量
void volume_down();//减小音量
void circle_play();//单曲循环
void sequence_play();//顺序播放
void singer_play(const char*s);
void shm_change();
void url_encode(char *dst, const char *src, int dst_size);



#endif
