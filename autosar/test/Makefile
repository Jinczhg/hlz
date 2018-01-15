#CC = QCC
#CFLAGS += -Vgcc_ntoarmv7le_gpp
CC = g++

TARGET = test
OBJ = test.o
LIBS +=
CFLAGS += -std=c++0x -I./include -I. -g
LDFLAGS += -lpthread -lautosar_cm -L./lib

all: $(TARGET) test_skeleton test_proxy

test_skeleton: test_skeleton.o com/myCompany/radar_skeleton.o
	$(CC) -o test_skeleton test_skeleton.o com/myCompany/radar_skeleton.o $(CFLAGS) $(LDFLAGS) $(LIBS)
	
test_proxy: test_proxy.o com/myCompany/radar_proxy.o
	$(CC) -o test_proxy test_proxy.o com/myCompany/radar_proxy.o $(CFLAGS) $(LDFLAGS) $(LIBS)
	
$(TARGET): $(OBJ) 
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LIBS)

%.o:%.cpp
	$(CC) $(CFLAGS) -Wall -c $(@D)/$(<F) -o $(@D)/$(@F)

clean:
	rm -f $(TARGET) $(OBJ) test_skeleton test_skeleton.o com/myCompany/radar_skeleton.o test_proxy test_proxy.o com/myCompany/radar_proxy.o
