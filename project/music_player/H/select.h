#ifndef SELECT_H
#define SELECT_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>   // 定义 errno
#include <signal.h>  // 定义 EINTR（部分系统在 <errno.h> 或 <unistd.h> 中）
#include <unistd.h>  // 部分系统中 EINTR 定义在此
#include <linux/input.h>  // 定义 struct input_event、EV_KEY、KEY_* 等常量
#include <sys/stat.h>     // 可选：若涉及文件状态
#include <fcntl.h>        // 可选：若涉及文件打开（如 open()）
#include "player.h"
#include "cJSON.h"
#define FD_COPY(from, to)  memcpy((to), (from), sizeof(fd_set))
void init_select();
void show();

void m_select();
void select_read_stdio();
void select_read_button();
void select_read_serial();
void select_read_socket();

#endif