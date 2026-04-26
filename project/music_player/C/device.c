#include "device.h"

extern int g_maxfd;
extern fd_set READSET;
int g_buttonfd=0;//按钮fd
int g_serialfd=0;//串口fd
int g_buzzerfd=0;//蜂鸣器fd

int init_device()
{
    set_volume(60);//初始化音量为60
    
#ifdef ARM
    //初始化按键
     g_buttonfd=open("/dev/input/event1",O_RDONLY);
    if(g_buttonfd<0)
    {
        perror("open buttons");
        return -1;
    }
    g_maxfd=(g_maxfd<g_buttonfd) ? g_buttonfd:g_maxfd;
    //加入集合中用于监听
    FD_SET(g_buttonfd,&READSET);

    //初始化串口
    g_serialfd = open("/dev/ttymxc2", O_RDWR | O_NOCTTY);//O_NOCTTY让程序（尤其是后台服务）免受来自该终端的意外信号干扰
    if(g_serialfd<0)
    {
        printf("出错\n");
        perror("open serilafd");
        return -1;
    }
    printf("串口初始化ok\n");
    init_serial(g_serialfd);
    g_maxfd=(g_maxfd<g_serialfd) ? g_serialfd:g_maxfd;
    FD_SET(g_serialfd,&READSET);


    //初始化蜂鸣器
    g_buzzerfd = open("/sys/class/leds/beep/brightness", O_RDWR | O_NONBLOCK);//O_NONBLOCK设置为非阻塞模式，就算蜂名气没有响也打开音箱
    if(g_buzzerfd<0)
    {
        perror("open buzzer");
        return -1;
    }
#endif
    return 0;
}

 // 建议函数返回int类型，表示成功或失败

    int init_serial(int fd) {
    int status;
    struct termios options;

    // 1. 获取当前串口属性
    if (tcgetattr(fd, &options) != 0) {
        perror("init_serial: tcgetattr failed");
        return -1;
    }

    // 2. 设置串口为原始模式
    cfmakeraw(&options);

    // 3. 设置波特率（语音模块常见115200，若模块是9600则改B9600）
    if (cfsetispeed(&options, B115200) != 0 || cfsetospeed(&options, B115200) != 0) {
        perror("init_serial: cfsetispeed/cfsetospeed failed");
        return -1;
    }

    // 4. 控制模式配置（核心）
    options.c_cflag |= (CLOCAL | CREAD); // 忽略调制解调器状态，启用接收
    options.c_cflag &= ~PARENB;          // 无奇偶校验
    options.c_cflag &= ~CSTOPB;          // 1位停止位
    options.c_cflag &= ~CSIZE;           // 清空数据位掩码
    options.c_cflag |= CS8;              // 8位数据位
    options.c_cflag &= ~CRTSCTS;         // 关闭硬件流控（关键：语音模块不用）

    // 5. 超时配置（适配select，改为阻塞读1个字节，无超时）
    options.c_cc[VMIN] = 1;   // 最少读1个字节才返回
    options.c_cc[VTIME] = 0;  // 无超时（由select控制超时）

    // 6. 应用配置
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        perror("init_serial: tcsetattr failed");
        return -1;
    }

    // 7. 清空缓冲区
    if (tcflush(fd, TCIOFLUSH) != 0) {
        perror("init_serial: tcflush failed (warning)"); // 警告而非致命错误
    }

    // 8. 关闭硬件流控（无需设置DTR/RTS，注释掉）
    // 语音模块不用硬件流控，这部分代码可删除
    // if (ioctl(fd, TIOCMGET, &status) == -1) { ... }

    usleep(10000); // 确保设置生效
    printf("串口初始化成功,fd==%d\n", fd);
    return 0;
}

//
void set_beep() 
{
    
    // 使用 write 写入数据
    int bytes_written= write(g_buzzerfd, "1", 1);
    if (bytes_written != 1) {
        perror("write");
        printf("蜂名气\n");
        close(g_buzzerfd);  // 写入失败时也要关闭文件
        return ;
    }
    sleep(1);
    bytes_written= write(g_buzzerfd, "0", 1);
    if (bytes_written != 1) {
        perror("write");
        close(g_buzzerfd);  // 写入失败时也要关闭文件
        return ;
    }
    // 关闭文件描述符
    close(g_buzzerfd);
    return ;
}

