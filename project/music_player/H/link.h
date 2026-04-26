#ifndef LINK_H
#define LINK_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "player.h"
typedef struct Node
{
    char music_name[128];
    struct Node*next;
    struct Node*prior;

}Node;
int init_link();// 初始化链表
void create_link(const char*s,const char*name);
int insert_link(const char*name);
void PriorMusic(const char *cur, int mode, char *prior);
void NextMusic(const char *cur, int mode, char *next);
void FindNextMusic(const char *cur, int mode, char *next);
void clear_link();


#endif
