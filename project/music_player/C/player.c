#include "player.h"
int g_shmid=-1; //共享内存句柄
void *g_addr = NULL;      //共享内存映射地址
int g_start_flag = 0;     //表示没有开始播放音乐
int g_suspend_flag = 0;   //表示没有暂停
extern Node *head;

int init_shm()
{
    // 创建共享内存
    g_shmid = shmget(SHMKEY, SHMSIZE, IPC_CREAT | IPC_EXCL | 0666); // 加上权限
    if (-1 == g_shmid)
    {
		printf("lll");
        perror("shmget");
		printf("pppp");
        // 如果是因为已存在而失败，可以尝试直接获取
       return -1;
    }
    // 共享内存映射
    g_addr = shmat(g_shmid, NULL, 0);
    if ((void*)-1 == g_addr)
    {
		printf("kkkkk");
        perror("init shmat");
		printf("mmmm\n");
        return -1;
    }
    Shm*s=(Shm*)malloc(sizeof(Shm));
	if(s==NULL)printf("NULL\n");
    memset(s, 0, sizeof(Shm));
	strcpy(s->cur_music,head->next->music_name);
    s->mode = SEQUENCE;
    memcpy(g_addr, s, sizeof(Shm)); // 把数据写入共享内存
    shmdt(g_addr);  

    return 0;
}
//获取共享内存数据
void  get_shm(Shm* s)
{
    if (g_shmid == -1) {
        fprintf(stderr, "错误：g_shmid未初始化（值为-1），无法映射共享内存\n");
        return ;
    }
    g_addr = shmat(g_shmid, NULL, 0);
    if ((void*)-1 == g_addr)
    {
        perror("get shmat");
        return ;
    }
    memcpy(s,g_addr,sizeof(Shm));
    shmdt(g_addr);
}

void get_singer(char*s)
{
	Shm s1;
	get_shm(&s1);
	char singer_str[128] = {0};
    
    // 提取歌手名（解决strchr的类型报错+使用s.cur_music）
    char *sep = strchr((const char*)s1.cur_music, '/');
    if (sep != NULL) {
        // 复制/前面的歌手名
        strncpy(singer_str,s1.cur_music, sep - s1.cur_music);
        // 手动加结束符，避免乱码
        singer_str[sep - s1.cur_music] = '\0';
	}
	s=singer_str;
}

void set_shm(Shm*s)
{
	 g_addr = shmat(g_shmid, NULL, 0);
    if ((void*)-1 == g_addr)
    {
        perror("get shmat");
		printf("2222\n");
        return ;
    }
    memcpy(g_addr,s,sizeof(Shm));
    shmdt(g_addr);
}

void set_volume(int volume) {
    char command[100];
    
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    
    snprintf(command, sizeof(command), "amixer -D pulse sset Master %d%% > /dev/null 2>&1", volume);
    system(command);
    
    printf("音量设置为: %d%%\n", volume);
}

int get_volume() {
    FILE *fp;
    char buffer[128];
    char command[] = "amixer -D pulse sget Master | grep -o '[0-9]*%' | head -1 | tr -d '%'";
    int volume = 50; // 默认值
    
    fp = popen(command, "r");
    if (fp == NULL) {
        printf("执行命令失败\n");
        return volume;
    }
    
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        volume = atoi(buffer);
    }
    
    pclose(fp);
    return volume;
}

void get_music(const char* s) //获取music
{
    // 发送请求：创建JSON对象
    cJSON* obj = cJSON_CreateObject();
    // 添加cmd字段："get_music_list"
    cJSON_AddStringToObject(obj, "cmd", "get_music_list");
    // 添加singer字段（若s为NULL则传空字符串）
    cJSON_AddStringToObject(obj, "singer", s);
    
    // 发送JSON数据（调用适配cJSON的socket_send_date函数）
    socket_send_date(obj);
    // 释放cJSON对象
    cJSON_Delete(obj);

    char msg[1024] = {0};
    socket_recv_date(msg);

    // 形成链表
    create_link(msg,s);

	//上传音乐数据
	upload_music_list();
}


// 核心函数：URL 编码（处理中文/空格/特殊字符，转为 UTF-8 格式的 %XX）
void url_encode(char *dst, const char *src, int dst_size) {
    if (dst == NULL || src == NULL || dst_size <= 0) {
        return;
    }

    int i = 0, j = 0;
    while (src[i] != '\0' && j < dst_size - 3) { // 预留 3 字节给 %XX
        // 普通字符（字母/数字/常用符号）直接复制
        if (isalnum((unsigned char)src[i]) || src[i] == '-' || src[i] == '_' || 
            src[i] == '.' || src[i] == '~') {
            dst[j++] = src[i];
        } 
        // 空格转为 %20（而非 +，兼容所有 HTTP 服务）
        else if (src[i] == ' ') {
            dst[j++] = '%';
            dst[j++] = '2';
            dst[j++] = '0';
        } 
        // 中文/其他多字节字符（UTF-8）转为 %XX 格式
        else {
            dst[j++] = '%';
            // 按 16 进制输出（大写，兼容标准）
            sprintf(&dst[j], "%02X", (unsigned char)src[i]);
            j += 2;
        }
        i++;
    }
    dst[j] = '\0'; // 确保字符串终止
}

//madplay播music
void play_music( char *name)
{
	pid_t child_pid = fork();
	if (-1 == child_pid)
	{
		perror("fork");
		exit(1);
	}
	else if (0 == child_pid)         //子进程
	{
		while (1)
		{
			pid_t grand_pid = fork();
			if (-1 == grand_pid)
			{
				perror("fork");
				exit(1);
			}
			else if (0 == grand_pid)    //孙进程
			{
				char cur_name[64] = {0};
				Shm s;
				//映射
				g_addr = shmat(g_shmid, NULL, 0);
				if (NULL == g_addr)
				{
					perror("shmat");
					exit(1);
				}

				if (strlen(name) != 0)      //直接开始播放
				{
					strcpy(cur_name, name);	
				}
				else                        //遍历链表，找到一首歌播放
				{
					//判断播放模式，找到一首歌曲
					memcpy(&s, g_addr, sizeof(s));
					FindNextMusic(s.cur_music, s.mode, cur_name);
				}

				//把信息写入共享内存（父子孙进程号、当前歌曲名）
				memcpy(&s, g_addr, sizeof(s));
				strcpy(s.cur_music, cur_name);
				s.child_pid = getppid();
				s.grand_pid = getpid();
				memcpy(g_addr, &s, sizeof(s));
				shmdt(g_addr);           //解除映射

				char music_path[328] = {0};     //包含路径的歌曲名称
				char encoded_name[328] = {0};
				url_encode(encoded_name, cur_name, sizeof(encoded_name));
				strcpy(music_path, MUSICPATH);
				strcat(music_path,encoded_name);
				printf("歌曲名字 %s\n", music_path);
				//execl("/usr/local/bin/madplay", "madplay", music_path, NULL);
				//execl("/usr/bin/mplayer", "mplayer", music_path, NULL);
                execl("/usr/bin/gst-play-1.0",
				"gst-play-1.0",
				"--no-interactive",
				"--audiosink=alsasink",        // 明确指定音频输出
				"--videosink=fakesink",        // 将视频输出设置为空
				music_path,
				(char *) NULL);

			}
			else                        //子进程
			{
				memset((void *)name, 0, strlen(name));         //歌曲名长度变为0，方便下一次操作

				int status;
				waitpid(grand_pid, &status, 0);        //回收孙进程
				//该歌曲播放完了
			}
		}
	}
	else
	{
		return;
	}
}

int start_play()
{
    if (g_start_flag == 1)     //已经开始播放
	{
		printf("kkkk\n");
		return -1;
	}

	//获取歌曲名称
	if (head->next == NULL)    //空链表
	{
		printf("mmmm\n");
		return -1;
	}
	printf("jjj\n");
	Shm s;
	memset(&s, 0, sizeof(s));
	g_addr = shmat(g_shmid, NULL, 0);
	memcpy(&s,g_addr,sizeof(Shm));
	char name[128] = {0};
	strcpy(name,s.cur_music);
	printf("音乐为：%s\n",name);
    // int  v=0;
    // get_volume(&v);
    // set_volume_system(v);
	play_music(name);
	g_start_flag = 1;
	return 0;

}


void stop_play()
{
	if (g_start_flag == 0)
	{
		return;
	}

	//读取共享内存，获取pid
	Shm s;
	memset(&s, 0, sizeof(s));
	g_addr = shmat(g_shmid, NULL, 0);
	memcpy(&s, g_addr, sizeof(s));

	//kill(s.grand_pid, SIGKILL);    //结束子进程
	//kill(s.child_pid, SIGKILL);    //结束孙进程
	
	
	kill(s.grand_pid, SIGKILL);      //结束孙进程
	kill(s.child_pid, SIGKILL);      //结束子进程
	
	g_start_flag = 0;
	int status;
	waitpid(s.child_pid,&status,0);
}

void suspend_play()
{
	if (g_start_flag == 0 || g_suspend_flag == 1)
	{
		return;
	}

	//读取共享内存，获取pid
	Shm s;
	memset(&s, 0, sizeof(s));
	g_addr = shmat(g_shmid, NULL, 0);
	memcpy(&s, g_addr, sizeof(s));

	kill(s.grand_pid, SIGSTOP);     //暂停孙进程
	kill(s.child_pid, SIGSTOP);     //暂停子进程

	g_suspend_flag = 1;
}
void continue_play()
{
	if (g_start_flag == 0 || g_suspend_flag == 0)
	{
		return;
	}

	//读取共享内存，获取pid
	Shm s;
	memset(&s, 0, sizeof(s));
	g_addr = shmat(g_shmid, NULL, 0);
	memcpy(&s, g_addr, sizeof(s));

	kill(s.grand_pid, SIGCONT); //恢复孙进程
	kill(s.child_pid, SIGCONT);//恢复子进程

	g_suspend_flag = 0;
}

void prior_play()
{
	if (g_start_flag == 0 || g_suspend_flag == 1)//还没有开始播放或者已经暂停了
	{
		return;
	}

	//读取共享内存，获取pid
	Shm s;
	memset(&s, 0, sizeof(s));
	g_addr = shmat(g_shmid, NULL, 0);
	memcpy(&s, g_addr, sizeof(s));

	//kill(s.grand_pid, SIGKILL);    //结束子进程
	//kill(s.child_pid, SIGKILL);    //结束孙进程
	
	
	kill(s.grand_pid, SIGKILL);      //结束孙进程
	kill(s.child_pid, SIGKILL);      //结束子进程

	g_start_flag = 0;
	int status;
	waitpid(s.child_pid,&status,0);

	char name[128] = {0};
	PriorMusic(s.cur_music,s.mode, name);
	play_music(name);	
	g_start_flag = 1;
}

void next_play()
{

	if (g_start_flag == 0 ||g_suspend_flag == 1)////还没有开始播放或者已经暂停了
	{
		return;
	}

	//读取共享内存，获取pid
	Shm s;
	memset(&s, 0, sizeof(s));
	g_addr = shmat(g_shmid, NULL, 0);
	memcpy(&s, g_addr, sizeof(s));

	//kill(s.grand_pid, SIGKILL);    //结束子进程
	//kill(s.child_pid, SIGKILL);    //结束孙进程
	g_start_flag = 0;

	char name[128] = {0};
	
	kill(s.grand_pid, SIGKILL);      //结束孙进程
	kill(s.child_pid, SIGKILL);      //结束子进程
	int status;
	waitpid(s.child_pid,&status,0);
	NextMusic(s.cur_music,s.mode, name);
	play_music(name);
	

	g_start_flag = 1;


}

//判断是不是.mp3结尾
int m_mp3_end(const char *name)
{
	const char *ptr = name;

	while (*ptr != '\0')
	{
		ptr++;
	}
	int i;
	for (i = 0; i < 4; i++)
	{
		ptr--;
	}
	
	if (ptr < name)
		return 0;
	
	return (strcmp(ptr, ".mp3") == 0) ? 1 : 0;
}

void volume_up() 
{
    int current_volume = get_volume();
    int new_volume = current_volume + 10;
    
    if (new_volume > 100) {
        new_volume = 100;
    }
    
    set_volume(new_volume);
    printf("音量增加至: %d%%\n", new_volume);
}

void volume_down() 
{
    int current_volume = get_volume();
    int new_volume = current_volume - 10;
    
    if (new_volume < 0) {
        new_volume = 0;
    }
    
    set_volume(new_volume);
    printf("音量降低至: %d%%\n", new_volume);
}
void circle_play()//单曲循环
{
   Shm s;
   get_shm(&s);
   s.mode=CIRCLE;
   set_shm(&s);
   printf("---单曲循环---\n");
}

void sequence_play()//顺序播放
{
   Shm s;
   get_shm(&s);
   s.mode=SEQUENCE;
   set_shm(&s);
   printf("---顺序播放\n");
}

void singer_play(const char*s)
{
	//停止播放
	stop_play();

	//清空链表
    clear_link();
	//获取歌曲
	get_music(s);

	//开始播放

    play_music(head->next->music_name);
	g_start_flag = 1;
}