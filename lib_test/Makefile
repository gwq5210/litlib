THIRDPARTY=../thirdparty
THIRDPARTY_INC=../thirdparty/include
THIRDPARTY_LIB_DIR=../thirdparty/lib
CURL_LIB=$(THIRDPARTY_LIB_DIR)/libcurl.a
JEMALLOC_LIB=$(THIRDPARTY_LIB_DIR)/libjemalloc.a
MYSQL_LIB=$(THIRDPARTY_LIB_DIR)/libmysqlclient.a
PROTOBUF_LIB=$(THIRDPARTY_LIB_DIR)/libprotobuf.a
CC=g++ -g -Wall -O2

################## Begin sys lib #############################
LIB = -pthread
INC = 
################## End sys lib ##############################

################## Begin thirdparty lib #############################
LIB += $(CURL_LIB) $(JEMALLOC_LIB) $(MYSQL_LIB) $(PROTOBUF_LIB)
INC += -I$(THIRDPARTY_INC)
################## End sys lib ##############################

FLG += 

BASEOBJ = \

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
	$(CC) -o $@ $(FLG) $(INC) $^ $(LIB)

%.o: %.cpp
	$(CC) $(INC) -c -o $@ $(FLG) $<

%.o: %.cc
	$(CC) $(INC) -c -o $@  $(FLG) $<
 
clean:
	rm -f ${USER_BIN} ${USER_OBJ}

cleanall:
	rm -f ${USER_BIN} ${USER_ALL_OBJ}
