#CC = QCC
#CFLAGS += -Vgcc_ntoarmv7le_gpp
CC = g++

TARGET = test
OBJ = test.o
LIBS +=
CFLAGS += -std=c++0x -I./include -g
LDFLAGS += -lpthread -lautosar_cm -L./lib

all: $(TARGET)

$(TARGET): $(OBJ) 
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LIBS)

%.o:%.cpp
	$(CC) $(CFLAGS) -Wall -c $(@D)/$(<F) -o $(@D)/$(@F)

clean:
	rm -f $(TARGET) $(OBJ)
