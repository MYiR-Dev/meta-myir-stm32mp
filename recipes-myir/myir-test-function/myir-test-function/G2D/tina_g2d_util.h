/**
 * @file tina_g2d_util.h
 *
 */

#ifndef TINA_G2D_UTIL_H
#define TINA_G2D_UTIL_H

#ifdef TINA_G2D

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "sunxi-g2d.h"
#include "ion_mem_alloc.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    G2D_ROTATE0 = 0,
    G2D_ROTATE90,
    G2D_ROTATE180,
    G2D_ROTATE270,
    G2D_FLIP_HORIZONTAL,
    G2D_FLIP_VERTICAL,
    G2D_MIRROR45,
    G2D_MIRROR135,
} g2dRotateAngle;

typedef struct
{
    int g2d_fd;
    int fp[4];
    int width;
    int height;
    int scale_w;
    int scale_h;
    int rotate_w;
    int rotate_h;
    int in_size;
    int out_size;
    void * src_addr;
    void * dst_addr;
    void * dst_addr_rt;
    void * dst_addr_fmat;
    struct SunxiMemOpsS * pMemops;
    g2d_blt_h info_fmat;
    g2d_blt_h info_rt;
    g2d_blt_h info;
    g2dRotateAngle angle;
    g2d_fmt_enh in_pixlformat;
} g2d_handler_t;
/**********************
 * GLOBAL PROTOTYPES
 **********************/
int g2d_init(g2d_handler_t * g2d_info);
int g2d_ops(g2d_handler_t * g2d_info, uint8_t * rgb_buf, uint8_t * scale_buf, bool* is_scale_changed);
void g2d_uninit(g2d_handler_t * g2d_info);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

#endif /*TINA_G2D_UTIL_H*/
