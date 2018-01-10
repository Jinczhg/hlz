#CC = QCC
#CFLAGS += -Vgcc_ntoarmv7le_gpp
CC = g++

TARGET = test
OBJ = test.o
LIBS +=
CFLAGS += -std=c++0x -I. -g
LDFLAGS += -lpthread -lautosar_cm -lzmq -L.

all: $(TARGET) test_server test_client

$(TARGET): $(OBJ) 
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LIBS)
	
test_server: test_server.o
	$(CC) -o test_server test_server.o $(CFLAGS) $(LDFLAGS) $(LIBS)
	
test_client: test_client.o
	$(CC) -o test_client test_client.o $(CFLAGS) $(LDFLAGS) $(LIBS)
	
%.o:%.cpp
	$(CC) $(CFLAGS) -Wall -c $(@D)/$(<F) -o $(@D)/$(@F)

clean:
	rm -f $(TARGET) $(OBJ) test_server test_client test_server.o test_client.o
