#include "main.h"

fd_set READSET;  // 用于监听fd的集合
int g_maxfd=0;
Node*head=NULL;
int com=0;
int main()
{

    //初始化select
    init_select();
    //printf("kjkj\n");

    //初始化设备文件
    if(init_device()==-1)
    {
        printf("初始化设备文件失败\n");
        //exit(1);
    }
    //printf("xiao\n");

    //初始化链表
    if(init_link()==-1)
    {
        printf("链表初始化失败\n");
        return -1;
    }



    //初始化网络(TCP协议)
    if(init_socket()==-1)
    {
        printf("网络初始化失败\n");
        return -1;
    }
    printf("连接服务器成功....\n");
    
     //初始化音乐
    get_music("其它");
     //初始化共享内存
    if(init_shm()==-1)
    {
        printf("初始化共享内存失败\n");
        return -1;
    }
    com=1;

    
    m_select();

    return 0;
}