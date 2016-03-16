#本项目相关变量
ROOT_DIR = .

#编译器
CXX = g++

#目标文件
TARGET = boost_regex jemalloc_test

THIRDPART = ../thirdpart

#头文件包含目录，一行一个
INC_DIR += -I${THIRDPART}/include

#静态库文件路径
STATIC_LIBS_DIR = ${THIRDPART}/lib
STATIC_LIBS = $(foreach n,$(STATIC_LIBS_DIR), $(wildcard $(n)/*.a)) 

#其他库文件，一行一个
STATIC_LIBS +=

#链接选项
LDFLAGS = -pthread
LDFLAGS += -ldl
LDFLAGS += ${STATIC_LIBS}

#编译选项
CFLAGS += -g -MMD -O2 -Wall -Wextra
CFLAGS += $(INC_DIR)

#自动搜寻，当前项目的目标文件
#SUBDIR可以指定多个目录，指定的目录下的所有cpp文件会加入编译，比如 SUBDIR = . src
SUBDIR = .
SUBDIR += ../utils
CURR_SOURCES =$(foreach n,$(SUBDIR), $(wildcard $(n)/*.cpp)) 

#其他外部依赖cpp文件在此处加上
CURR_SOURCES +=

#生成对应的.o文件
CXX_OBJECTS = $(patsubst %.cpp, %.o, ${CURR_SOURCES})

#添加自定义.o，这些.o不由clean删除，没有则无需填写
EXT_OBJECTS +=

#生成依赖关系
DEP_FILES += $(patsubst %.o, %.d, ${CXX_OBJECTS})
 
all: ${TARGET}
boost_regex:boost_regex.o
	rm -f $@
	${CXX} -o $@ $^ ${CFLAGS} ${LDFLAGS}
jemalloc_test:jemalloc_test.o
	rm -f $@
	${CXX} -o $@ $^ ${CFLAGS} ${LDFLAGS}

%.o:%.cpp
	${CXX} -c $(CFLAGS) -MT $@ -MF $(patsubst %.cpp, %.d,  $<) -o $@ $<
 
-include ${DEP_FILES}

test:
	@echo ${CURR_SOURCES}
 
clean:
	rm -rf ${TARGET} ${CXX_OBJECTS} ${DEP_FILES}
