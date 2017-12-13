CC = QCC
CFLAGS += -Vgcc_ntoarmv7le_gpp

TARGET = test
OBJ = test.o
LIBS +=
CFLAGS += -std=c++0x
LDFLAGS += 

all: $(TARGET)

$(TARGET): $(OBJ) 
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LIBS)

%.o:%.cpp
	$(CC) $(CFLAGS) -Wall -c $(@D)/$(<F) -o $(@D)/$(@F)

clean:
	rm $(TARGET) $(OBJ)
