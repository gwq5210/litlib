THIRDPARTY=../thirdparty
THIRDPARTY_INC=../thirdparty/include
THIRDPARTY_LIB_DIR=../thirdparty/lib
# make ssl
CURL_LIB=$(THIRDPARTY_LIB_DIR)/libcurl.a
JEMALLOC_LIB=$(THIRDPARTY_LIB_DIR)/libjemalloc.a
MYSQL_LIB=$(THIRDPARTY_LIB_DIR)/libmysqlclient.a
PROTOBUF_LIB=$(THIRDPARTY_LIB_DIR)/libprotobuf.a
UTIL_DIR=../utils
CC=g++ -g -Wall

################## Begin sys lib #############################
LIB = -pthread -ldl
INC = 
################## End sys lib ##############################

################## Begin thirdparty lib #############################
LIB += $(CURL_LIB) $(JEMALLOC_LIB) $(MYSQL_LIB) $(PROTOBUF_LIB)
INC += -I$(THIRDPARTY_INC)
INC += -I$(THIRDPARTY_INC)/mysql
################## End sys lib ##############################

################## Begin util lib #############################
LIB += 
INC += -I$(UTIL_DIR)
################## End sys lib ##############################

FLG += 
TEST_FLG += -DSDS_TEST
TEST_FLG += -DCURL_HELPER_TEST
TEST_FLG += -DEPOLL_HELPER_TEST
TEST_FLG += -DCOMMON_TOOL_TEST
TEST_FLG += -DMYSQL_HELPER_TEST
TEST_FLG += -DCFG_HELPER_TEST
TEST_FLG += -DREGEX_HELPER_TEST
TEST_FLG += -DCLIENT_HELPER_TEST
TEST_FLG += -DSERVER_HELPER_TEST
TEST_FLG += -DTHREAD_HELPER_TEST
TEST_FLG += -DSHM_HELPER_TEST
TEST_FLG += -DLIST_TEST

BASEOBJ = \
$(UTIL_DIR)/sds.o \
$(UTIL_DIR)/curl_helper.o \
$(UTIL_DIR)/epoll_helper.o \
$(UTIL_DIR)/common_tool.o \
$(UTIL_DIR)/mysql_helper.o \
$(UTIL_DIR)/cfg_helper.o \
$(UTIL_DIR)/regex_helper.o \
$(UTIL_DIR)/client_helper.o \
$(UTIL_DIR)/server_helper.o \
$(UTIL_DIR)/thread_helper.o \
$(UTIL_DIR)/shm_helper.o \
$(UTIL_DIR)/list.o

################## Begin User definitions ####################
## define process name
USER_BIN = lib_test
USER_OBJ =  \
jemalloc_test.o	\
lib_test.o

USER_ALL_OBJ = $(USER_OBJ) \
$(BASEOBJ)

all:
	$(MAKE) $(USER_BIN)

$(USER_BIN): ${USER_ALL_OBJ}
	rm -f $@
	$(CC) -o $@ $(TEST_FLG) $(FLG) $(INC) $^ $(LIB)

%.o: %.cpp
	$(CC) $(INC) -c -o $@ $(TEST_FLG) $(FLG) $<

%.o: %.cc
	$(CC) $(INC) -c -o $@ $(TEST_FLG) $(FLG) $<
 
clean:
	rm -f ${USER_BIN} ${USER_OBJ}

cleanall:
	rm -f ${USER_BIN} ${USER_ALL_OBJ}
