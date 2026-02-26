/**
 * @file camera_view.h
 *
 */

 #ifndef CAMERA_VIEW_H
 #define CAMERA_VIEW_H
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /*********************
  *      INCLUDES
  *********************/
#include <linux/types.h>
#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>
#ifdef TINA_G2D
#include "tina_g2d_util.h"
#endif
 /*********************
  *      DEFINES
  *********************/
#define FRAMEBUFFER_COUNT 4
#ifdef USE_ALLWINNER_ISP
// allwinner csi camere 
#define V4L2_MODE_VIDEO			0x0002
#define VIDIOC_SET_SENSOR_ISP_CFG \
_IOWR('V', BASE_VIDIOC_PRIVATE + 15, struct sensor_isp_cfg)

struct sensor_isp_cfg {
    __u8 isp_wdr_mode;
    __u8 large_image;
};
#endif

#define IMG_W 640
#define IMG_H 480
#define CLEAR(x) memset(&(x), 0, sizeof(x))
 /**********************
  *      TYPEDEFS
  **********************/
typedef  struct {
    void *  start;      //单平面内存映射的地址
    size_t  length;
} camera_buf_t;

typedef struct {
    int fd;
    camera_buf_t buf[FRAMEBUFFER_COUNT];
#ifdef TINA_G2D
    g2d_handler_t * g2d_info;
#endif
    int width;
    int height;
    int scale_w;
    int scale_h;
    int fb_width;
    int fb_height;
    int fb_width_byte;
    int dis_width_byte;
    pthread_t tid;
    uint64_t fps;
    uint64_t convtoms;
    __u32 pixelformat;
    __u32 capabilities;
    __u32 fmt_type;
    __u32 line_length;
    uint8_t * rgb_buf;
    uint8_t * scale_buf;
    uint8_t * display_buf;
    char dev_name[32];
    bool is_mplane;
    bool g_stop;
    bool ready;
    bool is_scale;
    bool is_scale_changed;
    bool is_rotate_changed;
    bool is_g2d_on;
    bool mutex_initialized;
    pthread_mutex_t mutex;
} v4l2_cam_t;
 
 /**********************
  * GLOBAL PROTOTYPES
  **********************/
int start_camera_thread(v4l2_cam_t *cam, const char * camera_port);
void exit_camera(v4l2_cam_t * cam);
 /**********************
  *      MACROS
  **********************/
 
 #ifdef __cplusplus
 } /*extern "C"*/
 #endif
 
 #endif /*CAMERA_VIEW_H*/
 