MAPPING = mapping
MONTAGE = montage

OBJ = cv.o mapping.o

ROOT_DIR=$(CURDIR)
OPENCV_INC=$(ROOT_DIR)/opencv-3.0.0/include/
OPENCV_LIB=$(ROOT_DIR)/opencv-3.0.0/lib/

CFLAGS=-std=c++11 -I$(OPENCV_INC)
LDFLAGS=-L$(OPENCV_LIB) -lopencv_core -lopencv_highgui -lopencv_calib3d -lopencv_imgcodecs -lopencv_imgproc -lopencv_videoio -lopencv_features2d

all: $(MAPPING) $(MONTAGE)

$(MAPPING): $(OBJ)
	g++ -o $(MAPPING) $(OBJ) $(LDFLAGS)

$(MONTAGE): montage.o
	g++ -o $(MONTAGE) montage.o

%.o:%.c
	g++ $(CFLAGS) -c $(@D)/$(<F) -o $(@D)/$(@F) -O2

%.o:%.cpp
	g++ $(CFLAGS) -c $(@D)/$(<F) -o $(@D)/$(@F) -O2

.PHONY: clean

clean:
	rm -f $(MAPPING) $(MONTAGE) $(MONTAGE).o $(OBJ)
