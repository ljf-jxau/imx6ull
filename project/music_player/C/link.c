#include "link.h"
#include "cJSON.h"
extern Node*head;

//存儲音樂链表初始化
int init_link()//带有头节点的双向链表的初始化
{
    printf("qqgg");
    head=(Node*)malloc(sizeof(Node));
    if(NULL==head)
    {
        printf("\n");
        return -1;
    }
    printf("0000");
    head->next=NULL;
    head->prior=NULL;
    return 0;
}


void create_link(const char* s,const char*name)
{
    // 1. 解析JSON字符串（替代json_tokener_parse）
    cJSON* obj = cJSON_Parse(s);
    if (obj == NULL) {
        printf("不是一个json格式的字符串\n");
        return;
    }

    // 2. 检查命令是否为"reply_music"（替代json_object_object_get_ex）
    cJSON* cmd_obj = cJSON_GetObjectItemCaseSensitive(obj, "cmd");
    if (!cJSON_IsString(cmd_obj) || (cmd_obj->valuestring == NULL)) {
        printf("未找到cmd字段或字段类型错误\n");
        cJSON_Delete(obj);
        return;
    }

    const char* cmd_str = cmd_obj->valuestring;
    if (strcmp(cmd_str, "reply_music") != 0) {
        printf("收到未知命令: %s\n", cmd_str);
        cJSON_Delete(obj);
        return;
    }
    printf("收到音乐列表回复命令\n");


   // 3. 取music数组前5个元素，直接插入链表（cJSON版本）
    cJSON* array = cJSON_GetObjectItemCaseSensitive(obj, "music");
    // 前置校验：确保array是有效数组（兼容原逻辑，无数组则直接退出）
    if (!cJSON_IsArray(array)) {
        printf("错误：music字段不存在或非数组类型\n");
        cJSON_Delete(obj); // 回收根对象
        return;
    }
    int i = 0;
     char music_name[1024]={0};
    // 原逻辑：循环5次，取数组前5个元素（即使数组长度不足5）
    for (i = 0; i < 5; i++) {
        // 替代 json_object_array_get_idx：获取数组第i个元素
        cJSON* music = cJSON_GetArrayItem(array, i);
        // 兼容：数组长度不足5时，music为NULL，跳过（避免崩溃）
        if (music == NULL || !cJSON_IsString(music) || music->valuestring == NULL) {
            printf("提示：music数组第%d个元素不存在/非字符串，跳过\n", i+1);
            continue;
        }
        strcpy(music_name,name);
        strcat(music_name,"/");
        strcat(music_name,music->valuestring);
        // 替代 json_object_get_string：获取字符串值并插入链表
        insert_link(music_name);
        memset(music_name,0,strlen(music_name));
    }
    // 回收cJSON资源（核心：根对象递归释放，包含所有子节点）
    cJSON_Delete(obj);
}
        

int insert_link(const char*name)
{
	if (NULL == name)
	{
		return -1;
	}
    Node*p=head;

	Node *n = (Node *)malloc(sizeof(Node) * 1);
	if (NULL == n)
	{
		return -1;
	}
    while(p->next)
    {
        p=p->next;
    }

	n->next =NULL;
	strcpy(n->music_name, name);
	p->next=n;
    n->prior=p;
	return 0;
}

void PriorMusic(const char *cur, int mode, char *prior)
{
	
		Node *p = head->next;
		while (strcmp(p->music_name, cur) != 0)
		{
			p = p->next;
		}

		if (p == head->next)
		{
			strcpy(prior, cur);
		}
		else
		{
			strcpy(prior, p->prior->music_name);
		}
		return;
}

void NextMusic(const char *cur, int mode, char *next)
{

		Node *p = head->next;
		while (strcmp(p->music_name, cur) != 0)
		{
			p = p->next;
		}
        if(p->next==NULL)
        {
            clear_link();
            char cur[100]={0};
            get_singer(cur);
            get_music(cur);
            strcpy(next, head->next->music_name);
            memset(cur,0,100);
        }
        else
        strcpy(next, p->next->music_name);
        return ;

}


void FindNextMusic(const char *cur, int mode, char *next)
{
	if (mode == CIRCLE)   //单曲循环
	{
		strcpy(next, cur);
		return;
	}
	else if (mode == SEQUENCE)
	{
		Node *p = head->next;
		while (strcmp(p->music_name, cur) != 0)
		{
			p = p->next;
		}
		if(p->next==NULL)
        {
            printf("没有音乐l了\n");
            clear_link();
            char cur[100]={0};
            get_singer(cur);
            get_music(cur);
            strcpy(next, head->next->music_name);
            memset(cur,0,100);
            return;
        }	

		strcpy(next, p->next->music_name);
		return;
	}
}

void clear_link()
{
    Node*p=head->next;
    Node*q=NULL;
    while(p)
    {
        q=p->next;
        free(p);
        p=q;
    }
    head->next=NULL;
    head->prior=NULL;
}