CC = QCC
CFLAGS += -Vgcc_ntoarmv7le

TARGET = libautosar_cm.so
OBJ = ManagementFactory.o SubscribeEvent.o Configuration.o PublishEvent.o ServiceProvider.o ServiceProxy.o ServiceRequester.o ServiceSkeleton.o SomeIpSd.o SomeIpBinding.o DataTypes.o Method.o Serializer.o Deserializer.o IpcBinding.o
LIBS += -lzmq
CFLAGS += -I. -I../include/someip -I../include/zmq
LDFLAGS += -L$(ROOT_DIR)/lib

all: $(TARGET)

$(TARGET): $(OBJ) 
	$(CC) -shared -o $(TARGET) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LIBS)
	cp libautosar_cm.so $(ROOT_DIR)/lib

%.o:%.cpp
	$(CC) $(CFLAGS) -Wall -fPIC -c $(@D)/$(<F) -o $(@D)/$(@F)

clean:
	rm -f $(TARGET) $(OBJ)
