#ifndef SOCKET_H
#define SOCKET_H
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "player.h"
#include "device.h"
#include "cJSON.h"  // cJSON头文件（需确保已包含）


#define PORT 8001
#define IP  "192.168.10.100"
int init_socket();
void socket_send_date(cJSON* j);
void socket_recv_date(char*msg);
void upload_music_list();
void socket_start_play();
void socket_stop_play();
void socket_suspend_play();
void socket_continue_play();
void socket_prior_play();
void socket_next_play();
void socket_voice_up();
void socket_voice_down();
void socket_circle_play();
void socket_sequence_play();

#endif