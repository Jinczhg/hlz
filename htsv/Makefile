BIN = sv
OBJ = main.o stitchalgo.o

LDFLAGS= -lopencv_core -lopencv_highgui

all: $(BIN)

$(BIN): $(OBJ)
	g++ -o $(BIN) $(OBJ) $(LDFLAGS)

%.o:%.c
	gcc $(CFLAGS) -DVERTICAL_SCREEN -c $(@D)/$(<F) -o $(@D)/$(@F) -O2 -g

%.o:%.cpp
	g++ $(CFLAGS) -DVERTICAL_SCREEN -c $(@D)/$(<F) -o $(@D)/$(@F) -O2 -g

.PHONY: clean

clean:
	rm -f $(BIN) $(OBJ)
