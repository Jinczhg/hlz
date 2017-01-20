#ifndef __m_BVCalib_H__
#define __m_BVCalib_H__
//#include "stitch.h"
int mBVCalib_Calib(  char* strCalibImage,
                                char* strInternalOutput,
                                char* strExternalOutput,
                                int CarWidth,int CarHeight,
                                int CarOffset,
                                int frontAngle, int rearAngle,
                                int birdViewHeight,
                                int cb_size,
                                int cb_hori,int cb_vert,
                                int cb2_hori,int cb2_vert,
                                int cb_XOffset,int cb_YOffset );
#endif
