include ./makerules

TARGET = sv_demo

OBJ = main.o dec.o avtp.o gpu.o stitch_2d.o stitch_3d.o yuv.o ipu_test.o stitchalgo.o

DEST_PATH=$(TARGET_ROOTFS)/bin

LDFLAGS += $(EXTRA_LDFLAGS)

EXTRA_CFLAGS += -DLINUX -DEGL_API_FB -DGL_GLEXT_PROTOTYPES

all: $(TARGET)

sv_demo: $(OBJ)
	$(CC) -lpthread -o sv_demo $(OBJ) $(LDFLAGS) -lvpu -lipu -lEGL -lGLESv2 -l_atbv_2_0_1 -lopencv_core -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_imgproc -lopencv_features2d -lopencv_legacy -lopencv_flann -lopencv_calib3d -lavcodec -lavformat -lavutil -lswscale -lswresample -lpng -lg2d

bv_demo: bv_demo.o
	$(CC) -lpthread -l_atbv_2_0_1 -o bv_demo bv_demo.o $(LDFLAGS) -lopencv_core -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_imgproc -lopencv_features2d -lopencv_legacy -lopencv_flann -lopencv_calib3d -lavcodec -lavformat -lavutil -lswscale -lswresample -lpng -lg2d

calib_int_single: calib_int_single.o
	$(CC) -lpthread -l_atbv_2_0_1 -o calib_int_single calib_int_single.o $(LDFLAGS) -lopencv_core -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_imgproc -lopencv_features2d -lopencv_legacy -lopencv_flann -lopencv_calib3d -lavcodec -lavformat -lavutil -lswscale -lswresample -lpng -lg2d
	
calib_int_merge: calib_int_merge.o
	$(CC) -lpthread -l_atbv_2_0_1 -o calib_int_merge calib_int_merge.o $(LDFLAGS) -lopencv_core -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_imgproc -lopencv_features2d -lopencv_legacy -lopencv_flann -lopencv_calib3d -lavcodec -lavformat -lavutil -lswscale -lswresample -lpng -lg2d

calib: calib.o
	$(CC) -lpthread -l_atbv_2_0_1 -o calib calib.o $(LDFLAGS) -lopencv_core -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_imgproc -lopencv_features2d -lopencv_legacy -lopencv_flann -lopencv_calib3d -lavcodec -lavformat -lavutil -lswscale -lswresample -lpng -lg2d

yuv2rgb: yuv2rgb.o yuvcvt.o
	$(CC) -o yuv2rgb yuv2rgb.o yuvcvt.o $(LDFLAGS) -lg2d

yuv_merge: yuv_merge.o yuv.o
	$(CC) -o yuv_merge yuv_merge.o yuv.o $(LDFLAGS)

ipu_test: ipu_test.o
	$(CC) -o ipu_test ipu_test.o $(LDFLAGS) -lipu
	
mxc_ipudev_test: mxc_ipudev_test.o utils.o
	$(CC) -o mxc_ipudev_test mxc_ipudev_test.o utils.o
	
opencv_test: TestOpencv.o
	$(CC) -o opencv_test TestOpencv.o $(LDFLAGS) -lopencv_core -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_imgproc -lopencv_features2d -lopencv_legacy -lopencv_flann -lopencv_calib3d -lavcodec -lavformat -lavutil -lswscale -lswresample
	
.PHONY: clean

clean:
	- rm -f $(TARGET) *.o

install: all
	sudo cp $(TARGET) $(DEST_PATH)

