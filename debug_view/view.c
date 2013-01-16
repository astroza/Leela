#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <highgui.h>
#include <arpa/inet.h>
#include <sys/socket.h>

IplImage  *frame;
/*
void rgb24_to_argb32(unsigned char *dst, unsigned char *src, int width, int height)
{
        int i;
        unsigned char *_dst = dst;
        unsigned char *_src = src;

        for(i = 0; i < width*height; i++) {
                *_dst++ = *_src++;
                *_dst++ = *_src++;
                *_dst++ = *_src++;
                *_dst++ = 0xff;
        }
}
*/
static char rgb24_buffer[320*240*3];
static char output[320*240*3];
int rgb24_buffer_count = 0;
#define min(a, b) (a < b? (a) : (b))
void recv_data_cb(char *data, int data_size)
{
        int data_offset = 0;
        int part_size;
        do {
                part_size = min(data_size, sizeof(rgb24_buffer) - rgb24_buffer_count);
                memcpy(rgb24_buffer+rgb24_buffer_count, data+data_offset, part_size);
                rgb24_buffer_count += part_size;
                if(rgb24_buffer_count == sizeof(rgb24_buffer)) {
                        //rgb24_to_argb32(output, rgb24_buffer, 320, 240);
			memcpy(output, rgb24_buffer, sizeof(output));
                        rgb24_buffer_count = 0;
			frame->imageData = output;
			cvShowImage("Imagen", frame);
                }
                data_offset += part_size;
                data_size -= part_size;
        } while(data_size > 0);

}

int main( int c, char **v )
{
    int fd;
    CvSize size;
    struct sockaddr_in sin;
    char buffer[320*240*3];
    int ret;
	int key;

    size.width = 320;
    size.height = 240;

    fd = socket(PF_INET, SOCK_STREAM, 0);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(3334);
    sin.sin_addr.s_addr = inet_addr("192.168.2.2");

    if(connect(fd, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
	puts("Error de conexion");
	exit(0);
    }

    /* create a window for the video */
    cvNamedWindow("Imagen", CV_WINDOW_AUTOSIZE);
    frame = cvCreateImageHeader( size, IPL_DEPTH_8U, 3);
    frame->imageData = output;

    while(1) {
	ret = recv(fd, buffer, sizeof(buffer), 0);
	if(ret <= 0) break;
	recv_data_cb(buffer, ret);
	   key = cvWaitKey(1);
    }

    /* free memory */
    cvDestroyWindow("Imagen");

    return 0;
}
