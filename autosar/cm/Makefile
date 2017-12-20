#CC = QCC
#CFLAGS += -Vgcc_ntoarmv7le_gpp
CC = g++

TARGET = test
OBJ = test.o ManagementFactory.o SubscribeEvent.o Configuration.o PublishEvent.o ServiceProvider.o ServiceProxy.o ServiceRequester.o ServiceSkeleton.o SomeIpSd.o SomeIpBinding.o DataTypes.o Method.o
LIBS +=
CFLAGS += -std=c++0x -I.
LDFLAGS += -lpthread

all: $(TARGET)

$(TARGET): $(OBJ) 
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LIBS)

%.o:%.cpp
	$(CC) $(CFLAGS) -Wall -c $(@D)/$(<F) -o $(@D)/$(@F)

clean:
	rm -f $(TARGET) $(OBJ)
