#ifndef __BVStitch_3D_2_0_H__
#define __BVStitch_3D_2_0_H__

#define  VIDEO_FORMAT_RGBA      0x1908
#define  VIDEO_FORMAT_YV12		0x8FC0
#define  VIDEO_FORMAT_NV12		0x8FC1
#define  VIDEO_FORMAT_YUY2		0x8FC2
#define  VIDEO_FORMAT_UYVY		0x8FC3
#define  VIDEO_FORMAT_NV21		0x8FC4




bool mBVStitch_2_0_Init(int fbnum,
                            char *szInter, char * szExter,
                            char * szCarMatrixFile, char * szCarModelFile,
                            int src_w, int src_h,int dst_x,int dst_y,int dst_w,int dst_h,float radratio);
void mBVStitch_2_0_ShutDown ();
void mBVStitch_2_0_StitchImage(unsigned char * pRGBA, int width, int height,int formatVideo);
bool mBVStitch_2_0_Draw(int bNeedCar);
bool mBVStitch_2_0_MoveCamera(int dx,int dy);
bool mBVStitch_2_0_MoveCameraAngle(float alpha, float beta,float vAngle);
bool mBVStitch_2_0_MoveCameraZoom(int dz);


#endif


