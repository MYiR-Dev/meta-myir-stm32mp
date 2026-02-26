/**
 * @file tina_g2d_util.c
 *
 */

 #ifdef TINA_G2D

/*********************
 *      INCLUDES
 *********************/
#include "tina_g2d_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

static int ion_memory_request(void ** vir_addr, unsigned int mem_size, struct SunxiMemOpsS * pMemops)
{
    int fd = -1, ret = 0;
    ret = SunxiMemOpen(pMemops);
    if(ret < 0) {
        printf("%s(%d): ION Open failed\n", __func__, __LINE__);
        return ret;
    }

    *vir_addr = SunxiMemPalloc(pMemops, mem_size);
    if(*vir_addr == NULL) {
        printf("%s(%d): ION Malloc failed\n", __func__, __LINE__);
        return -1;
    }
    fd = SunxiMemGetBufferFd(pMemops, *vir_addr);
    if(fd < 0) {
        printf("%s(%d): ION get dmabuf-fd failed\n", __func__, __LINE__);
        return -1;
    }
    printf("%s(%d): ION_IOC_ALLOC succes, dmabuf-fd = %d, size = %d\n", __func__, __LINE__, fd, mem_size);

    return fd;
}
 
static void ion_memory_release(void * virt_addr, struct SunxiMemOpsS * pMemops)
{
    SunxiMemPfree(pMemops, virt_addr);
}
 
static void ion_flush_cache(void * virt_addr, unsigned int mem_size, struct SunxiMemOpsS * pMemops)
{
    SunxiMemFlushCache(pMemops, virt_addr, mem_size);
}
 
int g2d_init(g2d_handler_t * g2d_info)
{
    int out_size;
    out_size = g2d_info->scale_w * g2d_info->scale_h * 4;
    if(G2D_FORMAT_YUV420UVC_V1U1V0U0 == g2d_info->in_pixlformat)
        g2d_info->in_size = g2d_info->width * g2d_info->height * 3 / 2;
    else g2d_info->in_size = g2d_info->width * g2d_info->height * 2;
    g2d_info->g2d_fd = open("/dev/g2d", O_RDWR);
    if(g2d_info->g2d_fd < 0) {
        printf("failed to open g2d device\n");
        return -1;
    }
    g2d_info->pMemops = GetMemAdapterOpsS();
    printf("pMemops is %p\n", g2d_info->pMemops);
    // camera output rgba src
    g2d_info->fp[0] = ion_memory_request(&g2d_info->src_addr, g2d_info->in_size, g2d_info->pMemops);
    if(g2d_info->fp[0] < 0) {
        printf("ion_memory_request src fd failed!\n");
        goto out1;
    }
    // scale output 
    g2d_info->fp[1] = ion_memory_request(&g2d_info->dst_addr, out_size, g2d_info->pMemops);
    if(g2d_info->fp[3] < 0) {
        printf("ion_memory_request scale dst fd failed!\n");
        goto out2;
    }
    
    // camera output yuyv src
    g2d_info->info.flag_h = (g2d_blt_flags_h)G2D_BLT_NONE_H;
    g2d_info->info.src_image_h.use_phy_addr = 0;
    g2d_info->info.src_image_h.fd           = g2d_info->fp[0];
    g2d_info->info.src_image_h.bbuff        = 1;
    g2d_info->info.src_image_h.mode         = G2D_PIXEL_ALPHA;
    g2d_info->info.src_image_h.alpha        = 0xff;
    g2d_info->info.src_image_h.format       = (g2d_fmt_enh)g2d_info->in_pixlformat;
    g2d_info->info.src_image_h.width        = g2d_info->width;
    g2d_info->info.src_image_h.height       = g2d_info->height;
    g2d_info->info.src_image_h.clip_rect.x  = 0;
    g2d_info->info.src_image_h.clip_rect.y  = 0;
    g2d_info->info.src_image_h.clip_rect.w  = g2d_info->width;
    g2d_info->info.src_image_h.clip_rect.h  = g2d_info->height;
    g2d_info->info.src_image_h.align[0]     = 0;
    g2d_info->info.src_image_h.align[1]     = 0;
    g2d_info->info.src_image_h.align[2]     = 0;

    // format to argb dst
    g2d_info->info.dst_image_h.use_phy_addr = 0;
    g2d_info->info.dst_image_h.fd           = g2d_info->fp[1];
    g2d_info->info.dst_image_h.bbuff        = 1;
    g2d_info->info.dst_image_h.mode         = G2D_PIXEL_ALPHA;
    g2d_info->info.dst_image_h.alpha        = 0xff;
    g2d_info->info.dst_image_h.format       = (g2d_fmt_enh)G2D_FORMAT_ARGB8888;
    g2d_info->info.dst_image_h.width        = g2d_info->scale_w;
    g2d_info->info.dst_image_h.height       = g2d_info->scale_h;
    g2d_info->info.dst_image_h.clip_rect.w  = g2d_info->scale_w;
    g2d_info->info.dst_image_h.clip_rect.h  = g2d_info->scale_h;
    g2d_info->info.dst_image_h.clip_rect.x  = 0;
    g2d_info->info.dst_image_h.clip_rect.y  = 0;
    g2d_info->info.dst_image_h.align[0]     = 0;
    g2d_info->info.dst_image_h.align[1]     = 0;
    g2d_info->info.dst_image_h.align[2]     = 0;


    return 0;

out2:
    ion_memory_release(g2d_info->src_addr, g2d_info->pMemops);
out1:
    close(g2d_info->g2d_fd);
    return -1;
}

int g2dformatConv(g2d_handler_t * g2d_info, uint8_t * inp_buf)
{
    int ret;
    memcpy(g2d_info->src_addr, inp_buf, g2d_info->in_size);
    ion_flush_cache(g2d_info->src_addr, g2d_info->in_size, g2d_info->pMemops);
    ret = ioctl(g2d_info->g2d_fd, G2D_CMD_BITBLT_H, (unsigned long)(&g2d_info->info_fmat));
    if(ret < 0) {
        printf("G2D_CMD_BITBLT_H error \n");
        return -1;
    }
    ion_flush_cache(g2d_info->dst_addr_fmat, g2d_info->out_size, g2d_info->pMemops);

    return 0;
}

int g2dRotate(g2d_handler_t * g2d_info, uint8_t * inp_buf)
{
    int in_size, out_size;
    int ret;
    out_size = g2d_info->width * g2d_info->height * 4;
    in_size = g2d_info->width * g2d_info->height * 4;

    // rotate
    switch(g2d_info->angle) {
        case G2D_ROTATE0: g2d_info->info_rt.flag_h = G2D_ROT_0; break;
        case G2D_ROTATE90: g2d_info->info_rt.flag_h = G2D_ROT_90; break;
        case G2D_ROTATE180:
            g2d_info->info_rt.flag_h = G2D_ROT_180; // G2D_BLT_ROTATE180
            break;
        case G2D_ROTATE270: g2d_info->info_rt.flag_h = G2D_ROT_270; break;
        case G2D_FLIP_HORIZONTAL: g2d_info->info_rt.flag_h = G2D_ROT_H; break;
        case G2D_FLIP_VERTICAL: g2d_info->info_rt.flag_h = G2D_ROT_V; break;
        case G2D_MIRROR45: g2d_info->info_rt.flag_h = G2D_BLT_MIRROR45; break;
        case G2D_MIRROR135: g2d_info->info_rt.flag_h = G2D_BLT_MIRROR135; break;
    }

    if(g2d_info->info_rt.flag_h == G2D_ROT_0) {
        g2d_info->info.src_image_h.fd           = g2d_info->fp[1];
        // ion_flush_cache(g2d_info->src_addr, in_size, g2d_info->pMemops);
        return 0;
    }else{
        g2d_info->info.src_image_h.fd           = g2d_info->fp[2];
    }

    g2d_info->info_rt.dst_image_h.width       = g2d_info->rotate_w;
    g2d_info->info_rt.dst_image_h.height      = g2d_info->rotate_h;
    g2d_info->info_rt.dst_image_h.clip_rect.w = g2d_info->rotate_w;
    g2d_info->info_rt.dst_image_h.clip_rect.h = g2d_info->rotate_h;
    
    ret = ioctl(g2d_info->g2d_fd, G2D_CMD_BITBLT_H, (unsigned long)(&g2d_info->info_rt));
    if(ret < 0) {
        printf("G2D_CMD_BITBLT_H error \n");
        return -1;
    }
    ion_flush_cache(g2d_info->dst_addr_rt, out_size, g2d_info->pMemops);

    return 0;
 }

static int dst_scale_buffer_get(g2d_handler_t * g2d_info, bool* is_scale_changed)
{
    if(!(*is_scale_changed)) return 0;
    if(g2d_info->dst_addr != NULL) ion_memory_release(g2d_info->dst_addr, g2d_info->pMemops);
    g2d_info->fp[3] = ion_memory_request(&g2d_info->dst_addr, g2d_info->out_size, g2d_info->pMemops);
    if(g2d_info->fp[3] < 0) {
        printf("ion_memory_request dst fd failed!\n");
        goto out2;
    }
    *is_scale_changed = false;
    return 0;

out2:
    ion_memory_release(g2d_info->dst_addr_rt, g2d_info->pMemops);
out1:
    ion_memory_release(g2d_info->src_addr, g2d_info->pMemops);
    close(g2d_info->g2d_fd);
    return -1;
}
 
int g2dScale(g2d_handler_t * g2d_info, bool * is_scale_changed)
{
    int ret;
    // scale

    ret = dst_scale_buffer_get(g2d_info, is_scale_changed);
    if(ret < 0) {
        printf("dst buffer get error \n");
        return -1;
    }
    
    ret = ioctl(g2d_info->g2d_fd, G2D_CMD_BITBLT_H, (unsigned long)(&g2d_info->info));
    if(ret < 0) {
        printf("G2D_CMD_BITBLT_H error \n");
        return -1;
    }
    ion_flush_cache(g2d_info->dst_addr, g2d_info->out_size, g2d_info->pMemops);
    // memcpy(cam->scale_buf, cam->g2d_info->dst_addr, out_size);

    return 0;
}
 
 int g2d_ops(g2d_handler_t * g2d_info, uint8_t * inp_buf, uint8_t * scale_buf, bool* is_scale_changed)
 {
    int ret;
    g2d_info->out_size = g2d_info->scale_w * g2d_info->scale_h * 4;

    memcpy(g2d_info->src_addr, inp_buf, g2d_info->in_size);
    ion_flush_cache(g2d_info->src_addr, g2d_info->in_size, g2d_info->pMemops);
    ret = g2dScale(g2d_info, is_scale_changed);
    if(ret < 0) {
        printf("g2dScale error \n");
        return -1;
    }
    memcpy(scale_buf, g2d_info->dst_addr, g2d_info->out_size);
 
    return 0;
}

void g2d_uninit(g2d_handler_t * g2d_info) {
    if(g2d_info == NULL) return;
    if(g2d_info->dst_addr_rt != NULL) {
        ion_memory_release(g2d_info->dst_addr_rt, g2d_info->pMemops);
        g2d_info->dst_addr_rt = NULL;
    }
    if(g2d_info->src_addr != NULL) {
        ion_memory_release(g2d_info->src_addr, g2d_info->pMemops);
        g2d_info->src_addr = NULL;
    }
    if(g2d_info->dst_addr != NULL) {
        ion_memory_release(g2d_info->dst_addr, g2d_info->pMemops);
        g2d_info->dst_addr = NULL;
    }
    if(g2d_info->dst_addr_fmat != NULL) {
        ion_memory_release(g2d_info->dst_addr_fmat, g2d_info->pMemops);
        g2d_info->dst_addr_fmat = NULL;
    }
        
    if(g2d_info->pMemops != NULL) {
        SunxiMemClose(g2d_info->pMemops);
        g2d_info->pMemops = NULL;
    }

    if(g2d_info->g2d_fd > 0) {
        close(g2d_info->g2d_fd);
        g2d_info->g2d_fd = -1;
    }
    
    return;
}
/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/
#endif