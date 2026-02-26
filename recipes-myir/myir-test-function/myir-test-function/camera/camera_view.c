#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <time.h>
#include <stdbool.h>
#include <poll.h>
#include <errno.h>
#include "camera_view.h"


static void * camera_thread_func(void * arg);
void uinit_mmap(v4l2_cam_t * cam);

static inline uint8_t clamp(int v)
{
    if(v < 0) return 0;
    if(v > 255) return 255;
    return v;
}

int xioctl(int fh, int request, void * arg)
{
    int r;
    do {
        r = ioctl(fh, request, arg);
    } while(-1 == r && EINTR == errno);

    return r;
}

uint64_t time_diff_ms(struct timeval start, struct timeval end)
{
    return (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
}

int v4l2_open(v4l2_cam_t * cam, const char * dev)
{
    struct v4l2_capability cap;
    cam->fd = open(dev, O_RDWR);
    if(cam->fd < 0) {
        perror("open camera");
        return -1;
    }

    if(ioctl(cam->fd, VIDIOC_QUERYCAP, &cap) < 0) {
        perror("VIDIOC_QUERYCAP");
        goto open_err_handle;
    }

    if((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)){
        cam->capabilities = 0x0 | V4L2_CAP_VIDEO_CAPTURE_MPLANE;
#ifdef USE_DEBUG
        fprintf(stdout, "%s set V4L2_CAP_VIDEO_CAPTURE_MPLANE\n", dev);
#endif
    }else if((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
        cam->capabilities = 0x0 | V4L2_CAP_VIDEO_CAPTURE;
#ifdef USE_DEBUG
        fprintf(stdout, "%s set V4L2_CAP_VIDEO_CAPTURE\n", dev);
#endif
    }else{
        fprintf(stdout, "%s don't support CAPTURE\n", dev);
        goto open_err_handle;
    }
    if((cap.capabilities & V4L2_CAP_STREAMING)) {
#ifdef USE_DEBUG
        fprintf(stdout, "%s support V4L2_CAP_STREAMING\n", dev);
#endif
    }else{
        fprintf(stdout, "%s don't support STREAMING I/O\n", dev);
        goto open_err_handle;
    }
    cam->is_mplane = (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) ? true : false;
    cam->fmt_type = cam->is_mplane ? V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE : V4L2_BUF_TYPE_VIDEO_CAPTURE;

    return 0;
open_err_handle:
    close(cam->fd);
    cam->fd = -1;
    return -1;
}

int v4l2_set_format(v4l2_cam_t * cam, int width, int height)
{
    struct v4l2_format fmt = {0};

#ifdef USE_ALLWINNER_ISP
    if(cam->is_mplane) {
        struct v4l2_input inp;
        struct v4l2_streamparm parms;
        struct sensor_isp_cfg sensor_isp_cfg;
    
        inp.index = 0;
        if (-1 == ioctl(cam->fd, VIDIOC_S_INPUT, &inp)) {
            printf("VIDIOC_S_INPUT error!\n");
            return -1;
        }
        CLEAR(parms);
        parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        parms.parm.capture.timeperframe.numerator = 1;
        parms.parm.capture.timeperframe.denominator = 0;
        parms.parm.capture.capturemode = V4L2_MODE_VIDEO;
        /* parms.parm.capture.capturemode = V4L2_MODE_IMAGE; */
        /* when different video have the same sensor source, 1:use sensor current win, 0:find the nearest win */
        parms.parm.capture.reserved[0] = 0;
        parms.parm.capture.reserved[1] = 0;/* 2:command, 1: wdr, 0: normal */
    
        if (-1 == ioctl(cam->fd, VIDIOC_S_PARM, &parms)) {
            printf("VIDIOC_S_PARM error\n");
            return -1;
        }
        CLEAR(sensor_isp_cfg);
        sensor_isp_cfg.isp_wdr_mode = 0;/* 2:command, 1: wdr, 0: normal */
        if (-1 == ioctl(cam->fd, VIDIOC_SET_SENSOR_ISP_CFG, &sensor_isp_cfg)) {
            printf("VIDIOC_SET_SENSOR_ISP_CFG error\n");
        }
    }
#endif
    if(cam->is_mplane) {
        fmt.type                   = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        fmt.fmt.pix_mp.width       = width;
        fmt.fmt.pix_mp.height      = height;
        fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12;
        fmt.fmt.pix_mp.field       = V4L2_FIELD_NONE;
        fmt.fmt.pix_mp.num_planes  = 1;
    } else {
        fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width       = width;
        fmt.fmt.pix.height      = height;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        fmt.fmt.pix.field       = V4L2_FIELD_NONE;
    }

    if(ioctl(cam->fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("VIDIOC_S_FMT");
        goto set_err_handle;
    }

    if(ioctl(cam->fd, VIDIOC_G_FMT, &fmt) < 0) {
        perror("VIDIOC_G_FMT");
        goto set_err_handle;
    }
#ifdef USE_DEBUG
    printf("w : %d, h : %d\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
#endif
    cam->width    = fmt.fmt.pix.width;
    cam->height   = fmt.fmt.pix.height;
    // cam->scale_w = cam->width;
    // cam->scale_h = cam->height;
    cam->pixelformat = cam->is_mplane ? V4L2_PIX_FMT_NV12 : V4L2_PIX_FMT_YUYV;
#ifdef TINA_G2D
    cam->g2d_info->rotate_w = cam->width;
    cam->g2d_info->rotate_h = cam->height;
    cam->g2d_info->scale_w = cam->scale_w;
    cam->g2d_info->scale_h = cam->scale_h;
    cam->g2d_info->width = cam->width;
    cam->g2d_info->height = cam->height;
    cam->g2d_info->in_pixlformat = cam->is_mplane ? G2D_FORMAT_YUV420UVC_V1U1V0U0 : G2D_FORMAT_IYUV422_V0Y1U0Y0;
#endif
    return 0;
set_err_handle:
    close(cam->fd);
    cam->fd = -1;
    return -1;
}

int v4l2_init_mmap(v4l2_cam_t * cam)
{
    struct v4l2_requestbuffers req = {0};
    struct v4l2_buffer buf         = {0};
    struct v4l2_plane mplanes[1];
    req.count  = FRAMEBUFFER_COUNT;
    req.type   = cam->fmt_type;
    req.memory = V4L2_MEMORY_MMAP;
    if(ioctl(cam->fd, VIDIOC_REQBUFS, &req) < 0) {
        perror("VIDIOC_REQBUFS");
        goto mmap_err_handle;
    }

    for(int i = 0; i < req.count; i++) {
        memset(&buf, 0, sizeof(buf));
        buf.type   = req.type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = i;
        if(cam->is_mplane) {
            buf.m.planes = mplanes;
            buf.length   = 1;
        }

        if(ioctl(cam->fd, VIDIOC_QUERYBUF, &buf) < 0) {
            perror("VIDIOC_QUERYBUF");
            goto mmap_err_handle;
        }

        size_t length = cam->is_mplane ? buf.m.planes[0].length : buf.length;
        off_t offset  = cam->is_mplane ? buf.m.planes[0].m.mem_offset : buf.m.offset;

        cam->buf[i].length = length;
        cam->buf[i].start  = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, cam->fd, offset);
        if(cam->buf[i].start == MAP_FAILED) {
            perror("mmap");
            goto mmap_err_handle1;
        }
#ifdef USE_DEBUG
        printf("BUF START %d: 0x%p\n", i, cam->buf[i].start);
#endif
        if(ioctl(cam->fd, VIDIOC_QBUF, &buf) < 0) {
            perror("VIDIOC_QBUF");
            goto mmap_err_handle1;
        }
    }
    return 0;
mmap_err_handle1:
    uinit_mmap(cam);
mmap_err_handle:
    close(cam->fd);
    cam->fd = -1;
    return -1;
}

int v4l2_start(v4l2_cam_t * cam)
{
    enum v4l2_buf_type type = cam->fmt_type;
    
    if(ioctl(cam->fd, VIDIOC_STREAMON, &type) < 0) {
        perror("VIDIOC_STREAMON");
        goto start_err_handle;
    }

    return 0;
start_err_handle:
    uinit_mmap(cam);
    close(cam->fd);
    cam->fd = -1;
    return -1;
}

int init_v4l2(v4l2_cam_t * cam, const char * camera_port)
{
    memset(cam->dev_name, 0, sizeof(cam->dev_name));
    strncpy(cam->dev_name, camera_port, strlen(camera_port));
    if(v4l2_open(cam, camera_port) < 0) {
        printf("can't open camera\n");
        return -1;
    }
    if(v4l2_set_format(cam, IMG_W, IMG_H) < 0) {
        printf("can't set format\n");
        return -1;
    }
    if(v4l2_init_mmap(cam) < 0) {
        printf("can't init mmap buffer\n");
        return -1;
    }
    if(v4l2_start(cam) < 0) {
        printf("can't start camera\n");
        return -1;
    }
    
    return 0;
}

int start_camera_thread(v4l2_cam_t * cam, const char * camera_port)
{
#ifdef TINA_G2D
    if(cam->g2d_info == NULL) {
        cam->g2d_info = (g2d_handler_t *)calloc(1, sizeof(g2d_handler_t));
        memset(&cam->g2d_info->info, 0, sizeof(g2d_blt_h));
    }
#endif

    if( camera_port != NULL) {
        if(init_v4l2(cam, camera_port) != 0) {
            printf("v4l2 camera %s init failed\n", camera_port);
            return -1;
        }
    }

#ifdef TINA_G2D
    if(g2d_init(cam->g2d_info) != 0) {
        printf("g2d init failed\n");
        return -1;
    }
#endif
    cam->g_stop = false;
    if(pthread_create(&cam->tid, NULL, camera_thread_func, cam) != 0) {
        perror("pthread_create");
        return -1;
    }
    return 0;
}

void uinit_mmap(v4l2_cam_t * cam)
{
    int i;
    for(i = 0; i < FRAMEBUFFER_COUNT; i++) {
        if(cam->buf[i].start == NULL || cam->buf[i].start == MAP_FAILED) continue;

        if(munmap(cam->buf[i].start, cam->buf[i].length) == -1) {
            perror("munmap()");
        }

        cam->buf[i].start = NULL;
        usleep(1000);
    }
    return ;
}

void stop_camera(v4l2_cam_t * cam)
{
    cam->g_stop = true;
    if(cam->fd >= 0) {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(cam->fd, VIDIOC_STREAMOFF, &type);
        close(cam->fd);
        cam->fd = -1;
    }
}

void exit_camera(v4l2_cam_t * cam)
{
    if(cam == NULL) return ;

    uint32_t i;
    cam->g_stop = true;
    if(cam->tid > 0)
        pthread_join(cam->tid, NULL);
    
    uinit_mmap(cam);

    if(cam->fd > 0) {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(cam->fd, VIDIOC_STREAMOFF, &type);
        close(cam->fd);
        cam->fd = -1;
    }

    cam->is_scale = false;
    cam->is_mplane = false;
    cam->is_scale_changed = false;
    cam->is_rotate_changed = false;
    cam->ready = false;
    // cam->is_g2d_on = false;
#ifdef TINA_G2D
    g2d_uninit(cam->g2d_info);
    if(cam->g2d_info != NULL) {
        free(cam->g2d_info);
        cam->g2d_info = NULL;
    }
#endif
    return ;
}

void v4l2_free(v4l2_cam_t ** cam)
{
    if(*cam != NULL) {
        free(*cam);
        *cam = NULL;
    }
}

void yuyv_to_rgb565(uint8_t *yuyv, uint16_t *rgb565, int width, int height)
{
    int total_pixels = width * height;
    int idx = 0;   // yuyv index (bytes)
    int out = 0;   // rgb565 index (pixels)

    // Process two pixels at a time (YUYV: 4 bytes → 2 pixels)
    while (idx < total_pixels * 2) {
        uint8_t y0 = yuyv[idx + 0];
        int8_t  u  = (int8_t)(yuyv[idx + 1] - 128);  // center at 0
        uint8_t y1 = yuyv[idx + 2];
        int8_t  v  = (int8_t)(yuyv[idx + 3] - 128);

        // Convert first pixel (y0, u, v)
        int r0 = y0 + (int)(1.402f * v);
        int g0 = y0 - (int)(0.344f * u + 0.714f * v);
        int b0 = y0 + (int)(1.772f * u);

        // Convert second pixel (y1, u, v)
        int r1 = y1 + (int)(1.402f * v);
        int g1 = y1 - (int)(0.344f * u + 0.714f * v);
        int b1 = y1 + (int)(1.772f * u);

        // Clamp to [0, 255]
        r0 = clamp(r0); g0 = clamp(g0); b0 = clamp(b0);
        r1 = clamp(r1); g1 = clamp(g1); b1 = clamp(b1);

        // Pack into RGB565: R[4:0] G[5:0] B[4:0]
        uint16_t pix0 = ((r0 >> 3) << 11) | ((g0 >> 2) << 5) | (b0 >> 3);
        uint16_t pix1 = ((r1 >> 3) << 11) | ((g1 >> 2) << 5) | (b1 >> 3);

        rgb565[out++] = pix0;
        rgb565[out++] = pix1;

        idx += 4;
    }
}

void yuyv_to_argb8888(uint8_t * yuyv, uint32_t * argb, int width, int height)
{
    int total_pixels = width * height;
    int idx          = 0; // yuyv index
    int out          = 0; // argb index

    while(idx < total_pixels * 2) // 2 bytes per pixel in YUYV
    {
        int y0 = yuyv[idx + 0];
        int u  = yuyv[idx + 1] - 128;
        int y1 = yuyv[idx + 2];
        int v  = yuyv[idx + 3] - 128;

        int r0 = y0 + 1.402f * v;
        int g0 = y0 - 0.344f * u - 0.714f * v;
        int b0 = y0 + 1.772f * u;

        int r1 = y1 + 1.402f * v;
        int g1 = y1 - 0.344f * u - 0.714f * v;
        int b1 = y1 + 1.772f * u;

        r0 = clamp(r0);
        g0 = clamp(g0);
        b0 = clamp(b0);
        r1 = clamp(r1);
        g1 = clamp(g1);
        b1 = clamp(b1);

        argb[out++] = 0xFF000000 | (r0 << 16) | (g0 << 8) | b0;
        argb[out++] = 0xFF000000 | (r1 << 16) | (g1 << 8) | b1;

        idx += 4;
    }
}

void nv12_to_argb(unsigned char * nv12, uint32_t * argb, int width, int height)
{
    const uint8_t* yPlane = nv12;
    const uint8_t* uvPlane = nv12 + width *height;

    for (int y = 0; y < height; y++) {  
        for (int x = 0; x < width; x++) {
            int yIdx = y * width + x;
            int uvIdx = ((y >> 1) * width + (x & ~1));

            int Y = yPlane[yIdx];
            int U = uvPlane[uvIdx] - 128;
            int V = uvPlane[uvIdx + 1] - 128;


            int R = clamp((298 * Y + 409 * V + 128) >> 8);
            int G = clamp((298 * Y - 100 * U - 208 * V + 128) >> 8);
            int B = clamp((298 * Y + 516 * U + 128) >> 8);

            argb[y * width + x] = 0xFF000000U | (R << 16) | (G << 8) | B;
        }
    }
}

void nv12_to_argb_fast(uint8_t *nv12, uint32_t *argb, int width, int height)
{
    uint8_t *yPlane  = nv12;
    uint8_t *uvPlane = nv12 + width * height;

    for (int y = 0; y < height; y += 2) {

        uint8_t  *y0 = yPlane + y * width;
        uint8_t  *y1 = y0 + width;
        uint8_t  *uv = uvPlane + (y >> 1) * width;

        uint32_t *out0 = argb + y * width;
        uint32_t *out1 = out0 + width;

        for (int x = 0; x < width; x += 2) {

            int U = uv[x]     - 128;
            int V = uv[x + 1] - 128;

            int r_uv = 409 * V;
            int g_uv = -100 * U - 208 * V;
            int b_uv = 516 * U;

            int Y;

            // (y, x)
            Y = 298 * y0[x];
            out0[x] = 0xFF000000 |
            (clamp((Y + r_uv) >> 8) << 16) |
            (clamp((Y + g_uv) >> 8) << 8)  |
                clamp((Y + b_uv) >> 8);

            // (y, x+1)
            Y = 298 * y0[x + 1];
            out0[x + 1] = 0xFF000000 |
                (clamp((Y + r_uv) >> 8) << 16) |
                (clamp((Y + g_uv) >> 8) << 8)  |
                    clamp((Y + b_uv) >> 8);

            // (y+1, x)
            Y = 298 * y1[x];
            out1[x] = 0xFF000000 |
            (clamp((Y + r_uv) >> 8) << 16) |
            (clamp((Y + g_uv) >> 8) << 8)  |
                clamp((Y + b_uv) >> 8);

            // (y+1, x+1)
            Y = 298 * y1[x + 1];
            out1[x + 1] = 0xFF000000 |
                (clamp((Y + r_uv) >> 8) << 16) |
                (clamp((Y + g_uv) >> 8) << 8)  |
                    clamp((Y + b_uv) >> 8);
        }
    }
}


void scale_argb8888(uint32_t * src, int src_w, int src_h, uint32_t * dst, int dst_w, int dst_h)
{
    for(int y = 0; y < dst_h; y++) {
        int src_y = y * src_h / dst_h;
        for(int x = 0; x < dst_w; x++) {
            int src_x          = x * src_w / dst_w;
            dst[y * dst_w + x] = src[src_y * src_w + src_x];
        }
    }
}

static void * camera_thread_func(void * arg)
{
    struct v4l2_buffer buf;
    struct v4l2_plane mplanes[1];
    int r;
    struct timeval t_start, t_now, t1, t2, t0, t3;
    volatile uint64_t frame_count = 0;

    gettimeofday(&t_start, NULL);

    v4l2_cam_t * cam = (v4l2_cam_t *)arg;
    struct pollfd Fds[1];
    Fds[0].fd     = cam->fd;
    Fds[0].events = POLLIN;
    
    while(!cam->g_stop) {
        r = poll(Fds, 1, 1000); // 监听摄像头数据
        if(r == 0) {
            fprintf(stderr, "pid: %lu poll I/O timeout\n", pthread_self());
            break;
        } else if(r < 0) {
            fprintf(stderr, "poll I/O err: %d\n", errno);
            break;
        }
        gettimeofday(&t0, NULL);
        CLEAR(buf);
        buf.type   = cam->fmt_type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.length = 1;
        if(cam->is_mplane)
            buf.m.planes = mplanes;

        if(xioctl(cam->fd, VIDIOC_DQBUF, &buf) == -1) {
            perror("VIDIOC_DQBUF");
            break;
        }

        if(buf.index >= FRAMEBUFFER_COUNT) {
            fprintf(stderr, "Invalid buffer index\n");
            break;
        }

        gettimeofday(&t1, NULL);
        if(!cam->is_g2d_on) {
            if(cam->is_mplane) nv12_to_argb_fast(cam->buf[buf.index].start, (uint32_t *)cam->rgb_buf, IMG_W, IMG_H);
            else yuyv_to_rgb565(cam->buf[buf.index].start, (uint16_t *)cam->rgb_buf, IMG_W, IMG_H);
            
        }else{
#ifdef TINA_G2D
            if(g2d_ops(cam->g2d_info, cam->buf[buf.index].start, cam->scale_buf, &cam->is_scale_changed) < 0) {
                printf("g2dops error \n");
                break;
            }
#endif
        }

        gettimeofday(&t2, NULL);
        for (int y = 0; y < cam->scale_h; y++) {
            // 从 fb 起始位置 + 行偏移 开始写
            uint8_t *dst = cam->display_buf + y * cam->line_length;
            memcpy(dst, (uint8_t *)cam->rgb_buf + y * cam->dis_width_byte, cam->dis_width_byte);
        }
        gettimeofday(&t3, NULL);

        frame_count++;
        gettimeofday(&t_now, NULL);
        if(time_diff_ms(t_start, t_now) >= 1000) {
#ifdef USE_DEBUG
            printf("%s VIDIOC_DQBUF : %lldms FPS: %lld g2d : %lldms wtfb : %lldms\n", cam->dev_name, time_diff_ms(t0,t1), frame_count, time_diff_ms(t1,t2) ,time_diff_ms(t2,t3));
#endif
            cam->fps      = frame_count;
            cam->convtoms = time_diff_ms(t1, t2);
            frame_count = 0;
            t_start     = t_now;
        }

        if(xioctl(cam->fd, VIDIOC_QBUF, &buf) == -1) {
            perror("VIDIOC_QBUF");
            break;
        }
    }

    return NULL;
}
