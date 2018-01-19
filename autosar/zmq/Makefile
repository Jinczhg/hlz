CC = QCC
CFLAGS += -Vgcc_ntoarmv7le

TARGET = test_zmq
OBJ = test_zmq.o
LIBS += -lzmq -lsocket
CFLAGS += -I./include
LDFLAGS += -L./lib

all: $(TARGET)

$(TARGET): $(OBJ) 
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LIBS)

%.o:%.cpp
	$(CC) $(CFLAGS) -Wall -c $(@D)/$(<F) -o $(@D)/$(@F)
	
%.o:%.c
	$(CC) $(CFLAGS) -Wall -c $(@D)/$(<F) -o $(@D)/$(@F)

clean:
	rm -f $(TARGET) $(OBJ)
