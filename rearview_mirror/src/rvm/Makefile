TARGET = rvm_demo
OBJ = main.o output.o dec.o gpu.o avb.o
LIBS += -ldce -lmemmgr -lmmrpc -lsharedmemallocator -lpcap -lsocket -lscreen -lGLESv2 -lEGL
CFLAGS += $(CFLAG) -I$(ROOT_DIR)/include -Dxdc_target_types__=qnx/targets/arm/std.h -DTRACE_LEVEL=1 -O2
LDFLAGS += -L$(ROOT_DIR)/lib -Dxdc_target_types__=qnx/targets/arm/std.h

all: $(TARGET)

$(TARGET): $(OBJ) 
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm $(TARGET) $(OBJ)
