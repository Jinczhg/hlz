CC = qcc
TARGET = sv_demo
OBJ = main.o output.o dec.o gpu.o avb.o stitchalgo.o
LIBS += -ldce -lmemmgr -lmmrpc -lsharedmemallocator -lpcap -lsocket -lscreen
CFLAGS += -Vgcc_ntoarmv7le -I./include -Dxdc_target_types__=qnx/targets/arm/std.h -DTRACE_LEVEL=0 -O2
LDFLAGS += -L./lib -Dxdc_target_types__=qnx/targets/arm/std.h

all: $(TARGET)

$(TARGET): $(OBJ) 
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm $(TARGET) $(OBJ)
