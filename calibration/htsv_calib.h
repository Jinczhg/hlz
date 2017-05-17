#ifndef __HTSV_CALIB_H
#define __HTSV_CALIB_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int x_offset;
    int y_offset;
} board_2d_offset;

typedef struct {
    board_2d_offset front;
    board_2d_offset rear;
    board_2d_offset left;
    board_2d_offset right;
} calib_2d_offset;

int htsv_2d_calib(char *config_file, char *camera_para_file, calib_2d_offset offset, char *front_file, char *rear_file, char *left_file, char *right_file, char *output_file);

#ifdef __cplusplus
}
#endif

#endif //__HTSV_CALIB_H
