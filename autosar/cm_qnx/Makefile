CC = QCC
CFLAGS += -Vgcc_ntoarmv7le
#CC = g++

TARGET = test
OBJ = test.o
LIBS +=
CFLAGS += -I. -g
LDFLAGS += -lpthread -lautosar_cm -lzmq -L.

all: $(TARGET) test_server test_client1 test_client2

$(TARGET): $(OBJ) 
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LIBS)
	
test_server: test_server.o
	$(CC) -o test_server test_server.o $(CFLAGS) $(LDFLAGS) $(LIBS)
	
test_client1: test_client1.o
	$(CC) -o test_client1 test_client1.o $(CFLAGS) $(LDFLAGS) $(LIBS)
	
test_client2: test_client2.o
	$(CC) -o test_client2 test_client2.o $(CFLAGS) $(LDFLAGS) $(LIBS)
	
%.o:%.cpp
	$(CC) $(CFLAGS) -Wall -c $(@D)/$(<F) -o $(@D)/$(@F)

clean:
	rm -f $(TARGET) $(OBJ) test_server test_client1 test_client2 test_server.o test_client1.o test_client2.o
