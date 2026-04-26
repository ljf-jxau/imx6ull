 #定义目标程序名
TARGET = test
CC=arm-linux-gnueabihf-gcc
# 定义源文件路径（C文件夹下的所有.c文件）
SRC_DIR = ./C
# 定义头文件路径（H文件夹下的所有.h文件）
INC_DIR = -I./H 

AA=-L/home/zd/cJSON-master


# 自动获取C文件夹下的所有.c文件
SOURCES = $(wildcard $(SRC_DIR)/*.c)
# 将.c文件对应生成.o目标文件（放在当前目录）
OBJECTS = $(patsubst $(SRC_DIR)/%.c, %.o, $(SOURCES))

# 编译选项：指定头文件目录、开启警告
CFLAGS = $(INC_DIR) -Wall -DARM

# 编译规则：将每个.c文件编译为.o文件
%.o: $(SRC_DIR)/%.c
	$(CC)  $(CFLAGS) -c $< -o $@

# 链接规则：将所有.o文件链接为目标程序
$(TARGET): $(OBJECTS)
	$(CC)  $(OBJECTS) -o $(TARGET)  -static -lcjson $(AA)

# 清理规则：删除.o文件和目标程序
.PHONY: clean
clean:
	rm -f $(OBJECTS) $(TARGET)
	./init.sh

#arm-linux-gnueabihf-gcc  -DARM
