BIN = calib_2d
OBJ = calib_2d.o htsv_calib.o ADAS_algorithm.o

LDFLAGS= -lopencv_core -lopencv_highgui -lopencv_calib3d -lopencv_imgproc

all: $(BIN)

$(BIN): $(OBJ)
	g++ -o $(BIN) $(OBJ) $(LDFLAGS)

%.o:%.c
	gcc $(CFLAGS) -DVERTICAL_SCREEN -c $(@D)/$(<F) -o $(@D)/$(@F) -O2 -g

%.o:%.cpp
	g++ $(CFLAGS) -DVERTICAL_SCREEN -c $(@D)/$(<F) -o $(@D)/$(@F) -O2 -g -std=c++11

.PHONY: clean

clean:
	rm -f $(BIN) $(OBJ)
