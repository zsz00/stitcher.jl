#include <opencv/highgui.h>

#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/videodev2.h>
#include <stdio.h>

#include <string.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>

#include <getopt.h>


#define UVC_DEV_VID	(0x15AA)
#define UVC_DEV_PID	(0x1555)

#define YUV_IMG_WIDTH	(2592)
#define YUV_IMG_HEIGHT  (1944)

#define RESIZE_IMG_WIDTH  (2592)
#define RESIZE_IMG_HEIGHT (1944)

#define Debug_Stderr_Pri(...)           {fprintf(stderr,__VA_ARGS__);}

#define VIDEO_FORMAT_YUYV	(0)
#define VIDEO_FORMAT_RAW	(1)

static int video_format = VIDEO_FORMAT_YUYV;
static int preview_flag = 1;

/* 保存摄像头内存映射后的内存地址和数据长度 */
struct buffer {
    char * start;
    unsigned int length;
};

struct v4l2_info{
    int fd;
    int width;
    int height;
    struct buffer* buffers;
};

#define MMAP_BUFF_CNT       3

unsigned char* bbf = NULL;
unsigned char* pTemp = NULL;

static struct v4l2_info info;

int v4l2_init(int fd)
{
    int cam_fd = fd;
    if (cam_fd == -1) {
        printf("error : %s\n", strerror(errno));
        return -1;
    }

    int width,height;

    /* 得到描述摄像头信息的结构体 */
    struct v4l2_capability cap;
    int rel = ioctl(cam_fd, VIDIOC_QUERYCAP, &cap);
    if ( rel == -1) {
        printf("error : %s\n", strerror(errno));
        //goto ERROR;
        close(cam_fd);
        return -1;
    }
    /* 判断改设备支不支持捕获图像和流输出功能 */
    if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE)
        printf("it's camer!\n");
    else {
        printf("it's not a camer!\n");
        //goto ERROR;
        close(cam_fd);
        return -1;
    }
    if ((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING)
        printf("it's stream device!\n");
    else {
        printf("it's not a stream device!\n");
        //goto ERROR;
        close(cam_fd);
        return -1;
    }

    printf("Driver Name : %s\n\
        Card Name : %s\n\
        Bus info : %s\n\
        Driver Version : %u.%u.%u \n",\
        cap.driver, cap.card, cap.bus_info,\
        (cap.version>>16)&0xff, (cap.version>>8)&0xff, (cap.version)&0xff);

    /* 得到摄像头采集图像的格式信息 */
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rel = ioctl(cam_fd, VIDIOC_G_FMT, &fmt);
    if (rel == -1) {
        printf("get fmt failed!\n");
        //goto ERROR;
        close(cam_fd);
        return -1;
    }

    width = YUV_IMG_WIDTH;//fmt.fmt.pix.width;
    height = YUV_IMG_HEIGHT;//fmt.fmt.pix.height;

    printf("\
        width : %d  height : %d\n\
        pixelformat : %d\n\
        field : %d\n\
        bytesperline : %d\n\
        sizeimage : %d\n\
        colorspace : %d\n\
        priv : %d\n",\
        fmt.fmt.pix.width,\
        fmt.fmt.pix.height,\
        fmt.fmt.pix.pixelformat,\
        fmt.fmt.pix.field, \
        fmt.fmt.pix.bytesperline, \
        fmt.fmt.pix.sizeimage, \
        fmt.fmt.pix.colorspace, \
        fmt.fmt.pix.priv);
    /* 得到摄像头所支持的所有格式 */
    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf(" - Support format : \n");
    while (ioctl(cam_fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1) {
        printf("\t%d.%s\n", fmtdesc.index+1, fmtdesc.description);
        fmtdesc.index++;
    }

#if 0
    /* DEFAULT FORMAT: MJPEG*/
    /* SET IMAGE FORMAT: V4L2_PIX_FMT_YUYV*/
    struct v4l2_format _format;
    _format.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    _format.fmt.pix.width=YUV_IMG_WIDTH;//WIDTH;
    _format.fmt.pix.height=YUV_IMG_HEIGHT;//HEIGHT;
    _format.fmt.pix.field=V4L2_FIELD_ANY;
    _format.fmt.pix.pixelformat=V4L2_PIX_FMT_YUYV;
    int _ret =ioctl(cam_fd,VIDIOC_S_FMT,&_format);
#endif

    /* 向摄像头申请一个数据帧队列 */
    struct v4l2_requestbuffers req;
    req.count = MMAP_BUFF_CNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    rel = ioctl(cam_fd, VIDIOC_REQBUFS, &req);
    if(rel < 0) {
        printf("request buffers error : %s\n", strerror(errno));
        //goto ERROR;
        close(cam_fd);
        return -1;
    } else
        printf("request buffers successed!\n");

    /* 申请存储图像缓冲区地址和长度的数组内存 */
    struct buffer * buffers = (struct buffer *)malloc(4*sizeof(struct buffer *));
    if (buffers == NULL ) {
        printf("malloc buffers err : %s\n", strerror(errno));
        //goto ERROR;
        close(cam_fd);
        return -1;
    }
    /* 将缓冲区的内存映射到用户空间 */
    int n_buffers = 0;
    for (; n_buffers < req.count; n_buffers++) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;
        if( -1 == ioctl(cam_fd, VIDIOC_QUERYBUF, &buf)) {  //获取缓冲帧的地址
            printf("set buf error : %s\n", strerror(errno));
            //goto ERROR;
            close(cam_fd);
            return -1;
        } else {
            printf("set buf success!\n");
        }
        buffers[n_buffers].length = buf.length;
        /* 映射内存空间 */
        buffers[n_buffers].start = (char*)mmap(NULL, buf.length, PROT_READ|PROT_WRITE, \
        MAP_SHARED, cam_fd, buf.m.offset);
        if (NULL == buffers[n_buffers].start) {
            printf("mmap error : %s\n", strerror(errno));
            //goto MAP_ERROR;
            printf("err:MAP_ERROR\n");
            for (n_buffers=0; n_buffers<MMAP_BUFF_CNT; n_buffers++) {
                if (buffers[n_buffers].start != NULL)
                    munmap(buffers[n_buffers].start, buffers[n_buffers].length);
            }
            printf("err:ERROR\n");
            close(cam_fd);
            return -1;
        } else
            printf("mmap success! address = %p\n",buffers[n_buffers].start);
            ioctl(cam_fd, VIDIOC_QBUF, &buf);
    }

    /* 开启视频流 */
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(cam_fd, VIDIOC_STREAMON, &type)){
        printf("stream on err.");
        //goto MAP_ERROR;
        printf("err:MAP_ERROR\n");
        for (n_buffers=0; n_buffers<MMAP_BUFF_CNT; n_buffers++) {
            if (buffers[n_buffers].start != NULL)
                munmap(buffers[n_buffers].start, buffers[n_buffers].length);
        }
        printf("err:ERROR\n");
        close(cam_fd);
        return -1;
    } else
        printf("start stream!\n");

    info.fd = cam_fd;
    info.width = width;
    info.height = height;
    info.buffers = buffers;

    return 0;
}

void process_image_raw(unsigned char * raw, unsigned char * temp, unsigned char * buf, int width, int height)
{
    int count;
    for(count = 0; count < width * height; count++)
	temp[count] = raw[2*count];

    IplImage* pRaw = cvCreateImageHeader(cvSize(width, height), 8, 1);
    IplImage* pRGB = cvCreateImageHeader(cvSize(width, height), 8, 3);
    pRaw->imageData = (char*)temp;
    pRGB->imageData = (char*)buf;

    //cvCvtColor(pRaw, pRGB, CV_BayerRG2BGR);
}

void process_image_yuv(unsigned char * yuv, unsigned char * buf, int length)
{
    int count;
    int y0,u0,y1,v0;
    int b, g, r;
    for (count=0; count<length/4; count++) {
        y0 = yuv[count*4+0];
        u0 = yuv[count*4+1];
        y1 = yuv[count*4+2];
        v0 = yuv[count*4+3];

        r = y0 + 1.140*(v0 - 128);
        g = y0 - 0.395*(u0 - 128) - 0.581*(v0 - 128);
        b = y0 + 2.032*(u0 - 128);

        if(b < 0) b = 0;
        if(b > 255) b = 255;

        if(g < 0) g = 0;
        if(g > 255) g = 255;

        if(r < 0) r = 0;
        if(r > 255)r = 255;

        buf[count*6+0] = b;
        buf[count*6+1] = g;
        buf[count*6+2] = r;

        r = y1 + 1.140*(v0 - 128);
        g = y1 - 0.395*(u0 - 128) - 0.581*(v0 - 128);
        b = y1 + 2.032*(u0 - 128);

        if(b < 0) b = 0;
        if(b > 255) b = 255;

        if(g < 0) g = 0;
        if(g > 255) g = 255;

        if(r < 0) r = 0;
        if(r > 255)r = 255;

        buf[count*6+3] = b;
        buf[count*6+4] = g;
        buf[count*6+5] = r;
    }
}


static int frm = 0;
int v4l2_capture()
{
    int ret = 0;
    FILE* fd;
    char szFileName[100];
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(info.fd, &fds);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000;

#if 1
    ret = select(info.fd+1, &fds, NULL, NULL, &tv);
    if (ret == -1) {
        printf(" error : listen failes\n");
    } else if (ret == 0) {
        printf(" time out !\n");
    } else {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        /* 得到一帧数据 */

        if (ioctl(info.fd, VIDIOC_DQBUF, &buf) != -1) {
            int width = info.width;
            int height = info.height;

	        printf("################ get frame size: %d  bytesused: %d  width: %d  height:  %d\n", buf.length, buf.bytesused, width, height);

            if(bbf != NULL) {
		        if(video_format == VIDEO_FORMAT_RAW) {
		            if(pTemp!= NULL)
		    	        ;//process_image_raw((unsigned char*)(info.buffers[buf.index].start), pTemp, bbf, width, height);
		        }else
                    ;//process_image_yuv((unsigned char*)(info.buffers[buf.index].start), bbf, info.buffers[buf.index].length);
	        }

            sprintf(szFileName, "%d.jpg", frm);
            fd = fopen(szFileName, "wb+");
            if(NULL == fd) printf("#######create file err..\n");
            ret = fwrite(info.buffers[buf.index].start, 1, buf.bytesused, fd);
            fclose(fd);
	    
	    memset(info.buffers[buf.index].start, 0, width*height*2);

            /* 将帧放回队列 */
            if (-1 == (ioctl(info.fd, VIDIOC_QBUF, &buf)))
                printf(" put in failed!\n");

        //    int32_t val = 0;
        //    int32_t _ret;

            frm++;
            
        //    printf("##### file size: %d\n", ret);

        } else
            printf(" get frame failed!\n");
    }
#endif
    return ret;
}

int v4l2_close()
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(info.fd, VIDIOC_STREAMOFF, &type);

    /* 解除内存映射，关闭文件描述符，程序结束 */
    int i;
    for (i=0; i<MMAP_BUFF_CNT; i++) {
        if (info.buffers[i].start != NULL)
            munmap(info.buffers[i].start, info.buffers[i].length);
    }

    close(info.fd);
}

////////////////////////////////////////////////////////
static int32_t dev_fd = 0;

int initDevice(char* dev_path)
{
    int result = 0;
    bool find_device = false;

    {
        printf("$$$ condif\n");
            
        if(0 == strlen(dev_path))
	        dev_fd = open("/dev/video0", O_RDWR);
        else
            dev_fd = open(dev_path, O_RDWR);
    }

    for(int32_t index = 0;
        index < 1;
        index++)
    {

        int32_t fd = dev_fd;

        if(fd<=0)
        {
            printf("ERROR opening V4L interface \n");

            continue;
        }
        else
        {
        }

        if(!v4l2_init(fd)) result++;
    }

    return result;
}


void closeDevice()
{
    if(dev_fd) {
        v4l2_close();

        dev_fd = 0;
    }
}


int main(int argc, char **argv)
{
    char dev[100];
    memset(dev, 0, sizeof(dev));

//    cvNamedWindow("Display");

    if(argc == 2) strcpy(dev, argv[1]);

    bbf = (unsigned char*)malloc(3*YUV_IMG_WIDTH*YUV_IMG_HEIGHT);

    if(video_format == VIDEO_FORMAT_RAW)
	    pTemp = (unsigned char*)malloc(YUV_IMG_WIDTH*YUV_IMG_HEIGHT);

    if(initDevice(dev) > 0) {
        usleep(1000*900);

        printf("init_v4l2 fd = 0x%x \n",info.fd);

//	    IplImage* pImage = cvCreateImageHeader(cvSize(YUV_IMG_WIDTH, YUV_IMG_HEIGHT), 8, 3);
//	    IplImage* pResizeImage = cvCreateImage(cvSize(RESIZE_IMG_WIDTH, RESIZE_IMG_HEIGHT), 8, 3);

//	    pImage->imageData = (char*)bbf;
	    printf(" -    src image size : (w:%d h%d)  \n", YUV_IMG_WIDTH, YUV_IMG_HEIGHT);
	    printf(" - resize image size : (w:%d h%d)  \n", RESIZE_IMG_WIDTH, RESIZE_IMG_HEIGHT);
	
	//    cvNamedWindow("Display");

	    while(preview_flag) {
	        v4l2_capture();
	        
            if(bbf != NULL) {
//		        cvResize(pImage, pResizeImage);
//		        cvShowImage("Display", pResizeImage);
		
//		        int k = cvWaitKey(30);
//		        if(k > 0) preview_flag = false;
	        } else
		        printf("bbf buffer is NULL. \n");
	    }

//	    cvReleaseImage(&pImage);
//	    cvReleaseImage(&pResizeImage);

	    printf("close uvc device. \n");
	    closeDevice();

    } else
	    printf("initDevice failed. \n");

    free(bbf);

    if(pTemp) free(pTemp);

    return 0;
}
