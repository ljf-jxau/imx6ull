#include "select.h"
extern fd_set READSET;
extern int g_maxfd;
extern int g_sockfd;
extern int g_buttonfd;
extern int g_serialfd;
void init_select()
{
FD_ZERO(&READSET);
FD_SET(0,&READSET);
}

void show()
{
	printf("1、开始播放\n");
	printf("2、结束播放\n");
	printf("3、暂停播放\n");
	printf("4、继续播放\n");
	printf("5、下一首\n");
	printf("6、上一首\n");
	printf("7、增加音量\n");
	printf("8、减小音量\n");
	printf("9、单曲循环\n");
	printf("a、顺序播放\n");
}

void select_read_stdio()
{
    char ch;
    scanf("%c",&ch);
    printf("输入的值为：%c\n",ch);
    switch(ch)
    {
        case '1': printf("iiiii\n");start_play(); break;
        case '2': stop_play(); break;
        case '3': suspend_play(); break;//暂停
        case '4': continue_play(); break;//继续
        case '5': next_play(); break;//下一首
        case '6': prior_play(); break;//上一首
        case '7': volume_up(); break;//调高音量
        case '8': volume_down(); break;//调低音量
        case '9': circle_play(); break;//循环播放
        case 'a': sequence_play(); break; //顺序播放
    }
}

void select_read_button()
{
    struct input_event ev;
    read(g_buttonfd, &ev, sizeof(struct input_event));
    if (ev.type == EV_KEY)
    {
        if(ev.value == 0)
        start_play();
    }
}

void select_read_serial()
{
    printf("------串口读到数据------\n");
    char ch;
    if(read(g_serialfd,&ch,1)==-1)
    {
        perror("serial read");
        return ;
    }
    switch(ch)
    {
        case 0x01: start_play(); break;
        case 0x02: stop_play(); break;
        case 0x03: suspend_play(); break;
        case 0x04: continue_play(); break;
        case 0x05: prior_play(); break;
        case 0x06: next_play(); break;
        case 0x07: volume_up(); break;
        case 0x08: volume_down(); break;
        case 0x09: circle_play(); break;
        case 0x0a: sequence_play(); break; // 注意：'a' 是字符，其ASCII码可能不是你期望的按键ID
        case 0x0b: singer_play("周杰伦");break;
        case 0x0c:singer_play("汪苏泷");break;
    }
  
}

int parse_message(char* buf, char* cmd)
{
    // 解析JSON字符串（buf是输入的JSON数据，原代码中mag应为buf的笔误）
    cJSON* obj = cJSON_Parse(buf);
    if (NULL == obj)
    {
        printf("select.c 83 行 失败\n");
        return -1;
    }
    
    // 获取"cmd"字段的值
    cJSON* value = cJSON_GetObjectItem(obj, "cmd");
    if (NULL == value || cJSON_IsString(value) == 0) // 检查是否为字符串类型
    {
        printf("select.c 90 行 失败\n");
        cJSON_Delete(obj); // 释放JSON对象
        return -1;
    }
    
    // 拷贝字符串到cmd
    strcpy(cmd, value->valuestring);
    
    // 释放JSON对象
    cJSON_Delete(obj);
    
    return 0; 
}

void select_read_socket()
{
    printf("---收到服务器数据---");
    char cmd[128]={0};
    char buf[1024]={0};
    socket_recv_date(buf);
    if(parse_message(buf,cmd)==-1)
    {
        printf("收到的不是json格式\n");
    }
    printf("cmd %s\n",cmd);
    if(!strcmp(cmd,"app_start"))
    {
        socket_start_play();
    }
    else if(!strcmp(cmd,"app_stop"))
    {
        socket_stop_play();
    }
    else if(!strcmp(cmd,"app_suspend"))
    {
        socket_suspend_play();
    }
    else if(!strcmp(cmd,"app_continue"))
    {
        socket_continue_play();
    }
    else if(!strcmp(cmd,"app_prior"))
    {
        socket_prior_play();
    }
    else if(!strcmp(cmd,"app_next"))
    {
        socket_next_play();  
    }
    else if(!strcmp(cmd,"app_voice_up"))
    {
        socket_voice_up();
    }
    else if(!strcmp(cmd,"app_voice_down"))
    {
        socket_voice_down();
    }
    else if(!strcmp(cmd,"app_circle"))
    {
        socket_circle_play();
    }
    else if(!strcmp(cmd,"app_sequence"))
    {
        socket_sequence_play();
    }
    else if(!strcmp(cmd,"app_get_music"))
    {
        upload_music_list();
    }

}

void m_select()
{
    show();
    int ret;
    // char message[1024] = {0};
    // cJSON* json_obj = NULL; // 修改点 1: 使用 json-c 的结构体类型

    while (1)
    {
        fd_set tmpfd;
        FD_ZERO(&tmpfd);          // 先清空临时集合
        FD_COPY(&READSET, &tmpfd); // 用宏拷贝原始集合到临时集合
        ret = select(g_maxfd + 1, &tmpfd, NULL, NULL, NULL);
        if (-1 == ret && errno != EINTR)//select 调用是因真正的错误失败（如参数错误、文件描述符越界等）
        {
            printf("wm、\n");
            perror("select");
        }
        else if (-1 == ret && errno == EINTR)//select 并非真失败，而是被外部信号打断
        {
            continue;
        }

        if(FD_ISSET(0, &tmpfd))            //键盘
        {
            select_read_stdio();
        }   
        else if(FD_ISSET(g_buttonfd, &tmpfd))     //按键
        {
            select_read_button();

        }  
        else if(FD_ISSET(g_sockfd, &tmpfd))     //socket
        {
            select_read_socket();

        }   
        else if(FD_ISSET(g_serialfd, &tmpfd))     //语音(串口)
        {
            select_read_serial();
        }
    }
}