#ifndef DEVICE_H
#define DEVICE_H

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>   // 必须包含：定义termios结构体及相关函数（tcgetattr/tcsetattr等）
#include <unistd.h>
#include <sys/ioctl.h>   // 必须：定义TIOCMGET/TIOCMSET/TIOCM_DTR/TIOCM_RTS等宏
#include <termios.h>     // 串口配置相关（若未包含）
#include "player.h"


int init_device();//初始化设备
void set_beep();//蜂鸣器的开关
int init_serial(int fd) ;//串口的初始化

#endif 