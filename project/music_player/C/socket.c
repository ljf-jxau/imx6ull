#include "socket.h"

int g_sockfd=-1;   //socket文件描述符
extern fd_set READSET;
extern int g_maxfd;
extern Node*head;
extern int com;
extern int g_start_flag;
extern int g_suspend_flag;

//信号函数,每隔5秒向服务器上报数据（歌曲，音量，模式，设备号）

void send_server(int sig)
{
    if(com==1){//com为1表示已经获取到歌曲了
    // 创建JSON对象（替代json_object_new_object()）
    cJSON* SendObj = cJSON_CreateObject();
    
    // 添加字符串字段："cmd" : "info"（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(SendObj, "cmd", "info");
    
    // 获取当前播放的音乐
    Shm*s=(Shm*)malloc(sizeof(Shm));
    memset(s, 0, sizeof(Shm));
    get_shm(s);
    // 添加当前音乐字段（替代json_object_new_string）
    cJSON_AddStringToObject(SendObj, "cur_music", s->cur_music);
    
    // 获取当前的音乐模式（添加整数字段，替代json_object_new_int）
    cJSON_AddNumberToObject(SendObj, "mode", s->mode);

    // 播放状态
    if(g_start_flag==0)//表示还没有播放
    {
        cJSON_AddStringToObject(SendObj, "status", "stop");
    }
    else if(g_start_flag==1 && g_suspend_flag==1)//表示暂停播放了
    {
    cJSON_AddStringToObject(SendObj, "status", "suspend");
    }
    else if(g_start_flag==1 && g_suspend_flag==0)//表示已经开始播放了
    {
    cJSON_AddStringToObject(SendObj, "status", "start");
    }
    //获取当前系统的音量
    int volume;
    volume=get_volume();
    // 添加音量字段（整数，替代json_object_new_int）
    cJSON_AddNumberToObject(SendObj, "volume", volume);
    
    // 获取设备号
    cJSON_AddStringToObject(SendObj, "deviceid", "0001");

    // 直接传递 cJSON 对象（不再手动转字符串）
    socket_send_date(SendObj);  
    
    // 释放cJSON对象（替代json_object_put）
    cJSON_Delete(SendObj);
    free(s);
    }

    //printf("每两秒上传\n");
    alarm(2);  
}
/*
函数描述：向服务器发送json对象
发格式：json对象的长度 + json对象
函数参数：需要发送的对象
*/
void socket_send_date(cJSON* j)  // 参数改为cJSON*类型
{
    
    char msg[1024] = {0};
    // cJSON将对象转为字符串（cJSON_PrintUnformatted：无格式化，更紧凑）
    const char* s = cJSON_PrintUnformatted(j);
    if (s == NULL) {  // 检查字符串生成是否成功
        printf("cJSON转字符串失败\n");
        return;
    }
    
    int len = strlen(s);
    if (len + sizeof(int) >= 1024) {  // 防止缓冲区溢出
        printf("JSON数据过长，超出msg缓冲区\n");
        free((void*)s);  // cJSON_Print系列函数需手动释放字符串
        return;
    }
    
    memcpy(msg, &len, sizeof(int));         // 先发送长度
    memcpy(msg + sizeof(int), s, len);      // 再发送JSON数据
    
    if (send(g_sockfd, msg, len + sizeof(int), 0) == -1) {
        perror("send");
    }
    
    free((void*)s);  // 释放cJSON生成的字符串内存（关键！）
}

void socket_recv_date(char*msg)
{
    //获取长度
    int len;
    ssize_t size=0;
    while(1)
    {
        size+=recv(g_sockfd,msg+size,sizeof(int)-size,0);
        if(size>=sizeof(int))
        break;
    }
    len=*(int*)msg;
    printf("获取的长度为%d\n",len);
    size=0;
    memset(msg,0,1024);
    //获取数据
    while(1)
    {
        size+=recv(g_sockfd,msg+size,len-size,0);
        if(size>=len)
        break;
    }
    printf("获取的消息为：%s\n",msg);
}

int init_socket()
{
    int count=50;
    if (g_sockfd >= 0) {
        close(g_sockfd);
        g_sockfd = -1;
    }
    //创建socket
    g_sockfd=socket(AF_INET,SOCK_STREAM ,0);
    if(g_sockfd==-1)
    {
        perror("socket");
        return -1;
    }

   struct sockaddr_in server_info;
   memset(&server_info,0,sizeof(server_info));
   server_info.sin_family=AF_INET;
   //将主机字节序的短整型（16 位）转换为网络字节序
   server_info.sin_port=htons(PORT);
   //将点分十进制的 IPv4 字符串（如 "192.168.1.1"）转换为网络字节序的 32 位整数
   server_info.sin_addr.s_addr=inet_addr(IP);


    //向服务器发起连接
    while(count--)
    {
        int d;
        //connect连接成功返回0,失败返回-1
       if((d=connect(g_sockfd,(struct sockaddr *)&server_info,sizeof(server_info)))==-1)
       {
           printf("connect fail\n");
           sleep(1);
           continue;
       }
       if(d==0)
       {
           printf("连接成功\n");
       }
#ifdef ARM
       set_beep();//音箱连接成功打开蜂名气
#endif 
       //连接成功
       FD_SET(g_sockfd,&READSET);  //把socket加入到集合
       g_maxfd=(g_maxfd<g_sockfd) ? g_sockfd:g_maxfd;
       
       //音箱5秒上传一次数据
      struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    // 绑定信号处理函数
    sa.sa_handler =send_server;
    // 清空信号掩码：处理SIGALRM时不阻塞其他信号（可根据需求添加）
    sigemptyset(&sa.sa_mask);
    // 核心标志：SA_RESTART - 被信号打断的慢系统调用（select/poll）自动重启
    sa.sa_flags = SA_RESTART;//信号处理完成后，内核会自动重新执行被中断的系统调用

    // 注册SIGALRM信号,当sigaction的第3个参数为空时设置为自己的规则
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction register failed");
        return -1;
    }

       alarm(2);     //发送一次信号每隔2秒SIGALRM
       break;
    }
    return 0;
}

//更新链表后上传歌曲给app
void upload_music_list()
{
    // 创建JSON对象（替代json_object_new_object()）
    cJSON* obj = cJSON_CreateObject();
    
    // 添加字符串字段："cmd" : "upload_music"（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "cmd", "upload_music");

    // 创建JSON数组（替代json_object_new_array()）
    cJSON* array = cJSON_CreateArray();
    
    Node* p = head->next;
    while (p)
    {
        // 向数组添加字符串元素（替代json_object_array_add+json_object_new_string）
        cJSON_AddItemToArray(array, cJSON_CreateString(p->music_name));
        p = p->next;
    }

    // 将数组添加到JSON对象（替代json_object_object_add）
    cJSON_AddItemToObject(obj, "music", array);

    // 发送JSON对象（socket_send_date需适配cJSON*参数，若仍需字符串则用cJSON_Print）
    socket_send_date(obj);
    
    // 释放cJSON对象（替代json_object_put，cJSON_Delete会递归释放子对象）
    cJSON_Delete(obj);
}

void socket_start_play()
{
    // 创建JSON对象（替代json_object_new_object()）
    cJSON* obj = cJSON_CreateObject();
    
    // 添加字符串字段："cmd" : "app_start_reply"（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "cmd", "app_start_reply");
    
    if (start_play() == -1)
    {
        // 添加失败结果（替代json_object_object_add+json_object_new_string）
        cJSON_AddStringToObject(obj, "result", "failure");
    }
    else
    {
        // 添加成功结果（替代json_object_object_add+json_object_new_string）
        cJSON_AddStringToObject(obj, "result", "success");
    }
    
    // 发送JSON对象（socket_send_date需适配cJSON*参数，若需字符串则用cJSON_Print）
    socket_send_date(obj);
    
    // 释放cJSON对象（替代json_object_put）
    cJSON_Delete(obj);
}

void socket_stop_play()
{
     // 创建JSON对象（替代json_object_new_object()）
    cJSON* obj = cJSON_CreateObject();
    
    // 添加字符串字段："cmd" : "app_start_reply"（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "cmd", "app_stop_reply");
    
    stop_play();
        // 添加成功结果（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "result", "success");
    
    // 发送JSON对象（socket_send_date需适配cJSON*参数，若需字符串则用cJSON_Print）
    socket_send_date(obj);
    
    // 释放cJSON对象（替代json_object_put）
    cJSON_Delete(obj);
}

void socket_suspend_play()
{
      // 创建JSON对象（替代json_object_new_object()）
    cJSON* obj = cJSON_CreateObject();
    
    // 添加字符串字段："cmd" : "app_start_reply"（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "cmd", "app_suspend_reply");
    
    suspend_play();
        // 添加成功结果（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "result", "success");
    
    // 发送JSON对象（socket_send_date需适配cJSON*参数，若需字符串则用cJSON_Print）
    socket_send_date(obj);
    
    // 释放cJSON对象（替代json_object_put）
    cJSON_Delete(obj);



}

void socket_continue_play()
{
      // 创建JSON对象（替代json_object_new_object()）
    cJSON* obj = cJSON_CreateObject();
    
    // 添加字符串字段："cmd" : "app_start_reply"（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "cmd", "app_continue_reply");
    
    continue_play();
        // 添加成功结果（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "result", "success");
    
    // 发送JSON对象（socket_send_date需适配cJSON*参数，若需字符串则用cJSON_Print）
    socket_send_date(obj);
    
    // 释放cJSON对象（替代json_object_put）
    cJSON_Delete(obj);

}

void socket_prior_play()
{
       // 创建JSON对象（替代json_object_new_object()）
    cJSON* obj = cJSON_CreateObject();
    
    // 添加字符串字段："cmd" : "app_start_reply"（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "cmd", "app_prior_reply");
    
    prior_play();
        // 添加成功结果（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "result", "success");
    Shm s;
    get_shm(&s);

    cJSON_AddStringToObject(obj, "music", s.cur_music);
    
    // 发送JSON对象（socket_send_date需适配cJSON*参数，若需字符串则用cJSON_Print）
    socket_send_date(obj);
    
    // 释放cJSON对象（替代json_object_put）
    cJSON_Delete(obj);


}

void socket_next_play()
{
       // 创建JSON对象（替代json_object_new_object()）
    cJSON* obj = cJSON_CreateObject();
    
    // 添加字符串字段："cmd" : "app_start_reply"（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "cmd", "app_next_reply");
    
    next_play();
        // 添加成功结果（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "result", "success");
    Shm s;
    get_shm(&s);
    cJSON_AddStringToObject(obj, "music", s.cur_music); 
    // 发送JSON对象（socket_send_date需适配cJSON*参数，若需字符串则用cJSON_Print）
    socket_send_date(obj);
    
    // 释放cJSON对象（替代json_object_put）
    cJSON_Delete(obj);

}

void socket_voice_up()
{
      // 创建JSON对象（替代json_object_new_object()）
    cJSON* obj = cJSON_CreateObject();
    
    // 添加字符串字段："cmd" : "app_start_reply"（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "cmd", "app_voice_up");
    
    volume_up();
        // 添加成功结果（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "result", "success");
    int v;
    get_volume(&v);
    // 添加整型字段："music" : v（v为整数变量）
    cJSON_AddNumberToObject(obj, "voice", v);
    // 发送JSON对象（socket_send_date需适配cJSON*参数，若需字符串则用cJSON_Print）
    socket_send_date(obj);
    
    // 释放cJSON对象（替代json_object_put）
    cJSON_Delete(obj);

}

void socket_voice_down()
{
      // 创建JSON对象（替代json_object_new_object()）
    cJSON* obj = cJSON_CreateObject();
    
    // 添加字符串字段："cmd" : "app_start_reply"（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "cmd", "app_voice_up");
    
    volume_down();
        // 添加成功结果（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "result", "success");
    int v;
    get_volume(&v);
    // 添加整型字段："music" : v（v为整数变量）
    cJSON_AddNumberToObject(obj, "voice", v);
    // 发送JSON对象（socket_send_date需适配cJSON*参数，若需字符串则用cJSON_Print）
    socket_send_date(obj);
    
    // 释放cJSON对象（替代json_object_put）
    cJSON_Delete(obj);
}

void socket_circle_play()
{
     cJSON* obj = cJSON_CreateObject();
    
    // 添加字符串字段："cmd" : "app_start_reply"（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "cmd", "app_circle_reply");
    circle_play();
    cJSON_AddStringToObject(obj, "result", "success");
    // 发送JSON对象（socket_send_date需适配cJSON*参数，若需字符串则用cJSON_Print）
    socket_send_date(obj);
    
    // 释放cJSON对象（替代json_object_put）
    cJSON_Delete(obj);

}

void socket_sequence_play()
{
      cJSON* obj = cJSON_CreateObject();
    
    // 添加字符串字段："cmd" : "app_start_reply"（替代json_object_object_add+json_object_new_string）
    cJSON_AddStringToObject(obj, "cmd", "app_sequence_reply");
    sequence_play();
    cJSON_AddStringToObject(obj, "result", "success");
    // 发送JSON对象（socket_send_date需适配cJSON*参数，若需字符串则用cJSON_Print）
    socket_send_date(obj);
    
    // 释放cJSON对象（替代json_object_put）
    cJSON_Delete(obj);

}


