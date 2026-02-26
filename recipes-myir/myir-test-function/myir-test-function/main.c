#include "camera/camera_view.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_FB "/dev/fb0"

int main(int argc, char * argv[])
{
    const char camera_name1[32];
    const char camera_name2[32];
    int fd_fb;
    int g_fb_width, g_fb_height;
    size_t fb_size;
    uint8_t *fb_base;

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    if(argc < 2)
    {
        printf("fun camera_name\n");
        return -1;
    }

    fd_fb = open(DEVICE_FB,O_RDWR);
    if(fd_fb < 0){
        printf("Failed to open %s",DEVICE_FB);
        return -1;
    }

    if (ioctl(fd_fb, FBIOGET_FSCREENINFO, &finfo) < 0) {
        perror("FBIOGET_FSCREENINFO");
        close(fd_fb);
        return -1;
    }
    
    if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        perror("FBIOGET_VSCREENINFO");
        close(fd_fb);
        return -1;
    }

    g_fb_width = vinfo.xres;   
    g_fb_height = vinfo.yres;

    fb_size = finfo.line_length * vinfo.yres;

    fb_base = mmap(NULL, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);

    if (fb_base == MAP_FAILED) {
        perror("mmap fb");
        close(fd_fb);
        return -1;
    }

    v4l2_cam_t camera[2];
    for(int i = 0; i < argc-1; i++) {
        CLEAR(camera[i]);
        camera[i].is_scale = false;
        camera[i].is_g2d_on = false;
        camera[i].fb_width = g_fb_width;
        camera[i].fb_height = g_fb_height;
        camera[i].fb_width_byte = g_fb_width * 2;
        camera[i].scale_w = IMG_W;
        camera[i].dis_width_byte = camera[i].scale_w * 2;
        camera[i].scale_h = IMG_H;
        camera[i].rgb_buf = (uint8_t *)calloc(camera[i].scale_w * camera[i].scale_h * 2, sizeof(uint8_t));
        camera[i].display_buf = (uint8_t *)fb_base + (g_fb_width - camera[i].scale_w);
        camera[i].line_length = finfo.line_length;
    }
#ifdef USE_DEBUG
    for(int i = 0; i < argc-1; i++) {
        printf("scale_w: %d scale_h: %d fb_width_byte: %d dis_width_byte: %d fb_base: %x\n", camera[i].scale_w,
                camera[i].scale_h, camera[i].fb_width_byte, camera[i].dis_width_byte, camera[i].display_buf);
    }
    printf("Resolution: %dx%d\n", vinfo.xres, vinfo.yres);
    printf("Bits per pixel: %d\n", vinfo.bits_per_pixel);
    printf("Red:   length=%d, offset=%d\n", vinfo.red.length, vinfo.red.offset);
    printf("Green: length=%d, offset=%d\n", vinfo.green.length, vinfo.green.offset);
    printf("Blue:  length=%d, offset=%d\n", vinfo.blue.length, vinfo.blue.offset);
    printf("Transp:length=%d, offset=%d\n", vinfo.transp.length, vinfo.transp.offset);
    printf("line_length=%d\n", finfo.line_length);
    printf("fb_size=%zu\n", fb_size);
#endif

    // 写入纯红测试
    // uint16_t red = (31 << 11) | (0 << 5) | 0; // R=31 (max), G=0, B=0

    // uint16_t *rgb16 = (uint16_t *)camera[0].rgb_buf;
    // int total_pixels = camera[0].scale_w * camera[0].scale_h;

    // uint16_t red565 = 0xF800;  // (31 << 11) == 0xF800

    // for (int i = 0; i < total_pixels; i++) {
    //     rgb16[i] = red565;
    // }

    // for (int y = 0; y < camera[0].scale_h; y++) {
    //     // 从 fb 起始位置 + 行偏移 开始写
    //     uint8_t *dst = camera[0].display_buf + y * finfo.line_length;
    //     memcpy(dst, (uint8_t *)camera[0].rgb_buf + y * camera[0].dis_width_byte, camera[0].dis_width_byte);
    // }

    if(start_camera_thread(&camera[0], argv[1]) != 0) {
        perror("Failed to start camera1 thread");
    }

    for(int i = 0; i < 1; i++) {
        if(camera[i].tid > 0){
            if (pthread_join(camera[i].tid, NULL) != 0) {
                perror("pthread_join failed");
                return -1;
            }
        }
    }  

    return 0;
}