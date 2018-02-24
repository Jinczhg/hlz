CC = QCC
CFLAGS += -Vgcc_ntoarmv7le

SRC_DIR=../

LIBS += -lautosar_cm -lsocket -lzmq
CFLAGS += -I$(ROOT_DIR)/api/include -I$(SRC_DIR)
LDFLAGS += -L$(ROOT_DIR)/lib

all: test_skeleton test_proxy

test_skeleton: test_skeleton.o $(SRC_DIR)/com/myCompany/radar_skeleton.o
	$(CC) -o test_skeleton test_skeleton.o $(SRC_DIR)/com/myCompany/radar_skeleton.o $(CFLAGS) $(LDFLAGS) $(LIBS)
	
test_proxy: test_proxy.o $(SRC_DIR)/com/myCompany/radar_proxy.o
	$(CC)  $(CFLAGS) -o test_proxy test_proxy.o $(SRC_DIR)/com/myCompany/radar_proxy.o $(LDFLAGS) $(LIBS)

%.o:%.cpp
	$(CC) $(CFLAGS) -Wall -c $(@D)/$(<F) -o $(@D)/$(@F)

clean:
	rm -f test_skeleton test_proxy *.o $(SRC_DIR)/com/myCompany/*.o
