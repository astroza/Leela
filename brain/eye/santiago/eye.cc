/* (c) 2012, 2013 - Felipe Astroza Araya
 *
 *  This file is part of Leela.
 *
 *  Leela is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Leela is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Leela. If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <assert.h>
#include <sys/mman.h>

#include "yuyvrgb24.h"
#include "eye.h"

static const char eye_path[] = "/dev/video0";

// http://v4l2spec.bytesex.org/spec/capture-example.html
/* From V4L2 tutorial */
static int xioctl(int fd, int request, void *arg)
{
	int r;

	do r = ioctl (fd, request, arg);
	while (r == -1 && errno == EINTR); /* errno=EINTR suele suceder cuando
					    * se utiliza un depurador
					    */
	return r;
}

int Eye::init()
{
	struct v4l2_capability cap;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	struct v4l2_control ctl;
	struct v4l2_streamparm parm;
	unsigned int i;

	if (xioctl (this->fd, VIDIOC_QUERYCAP, &cap) == -1) {
		perror("ioctl");
		return -1;
        }

	if(!(cap.capabilities & V4L2_CAP_STREAMING))
		return -1; /* Camara equivocada? */

	/* No uso (no se puede) CROP en PS3 Eye */
	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = IMAGE_WIDTH;
	fmt.fmt.pix.height = IMAGE_HEIGHT;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;

	if(xioctl(this->fd, VIDIOC_S_FMT, &fmt) == -1) {
		perror("ioctl");
		return -1;
	}

	req.count = BUFFERS_COUNT;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (xioctl (this->fd, VIDIOC_REQBUFS, &req) == -1) {
		perror("ioctl");
		return -1;
        }

	assert(req.count > 2);
	this->yuyv_bufs_count = req.count;

	for(i = 0; i < req.count; i++) {
		memset(&buf, 0, sizeof(buf));
		// http://v4l2spec.bytesex.org/spec/r13022.htm
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		/* Prefiero terminar el programa que deshacer lo hecho. */
		assert(xioctl(this->fd, VIDIOC_QUERYBUF, &buf) != -1);

		this->yuyv_bufs[i].start = mmap(0, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, this->fd, buf.m.offset);
		this->yuyv_bufs[i].length = buf.length;

		/* Prefiero terminar el programa que deshacer lo hecho. */
		assert(this->yuyv_bufs[i].start != NULL);
	}

	ctl.id = V4L2_CID_VFLIP;
	ctl.value = 1;
	xioctl(this->fd, VIDIOC_S_CTRL, &ctl);

	ctl.id = V4L2_CID_HFLIP;
	ctl.value = 1;
	xioctl(this->fd, VIDIOC_S_CTRL, &ctl);

	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	xioctl(this->fd, VIDIOC_G_PARM, &parm);
	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = 30;
	xioctl(this->fd, VIDIOC_S_PARM, &parm);

	return 0;
}


int Eye::set_exposure(unsigned int value)
{
	struct v4l2_control ctl;
	int ret;

        ctl.id = V4L2_CID_EXPOSURE;
        ctl.value  = value;
	ret = xioctl(this->fd, VIDIOC_S_CTRL, &ctl);
	if(ret != -1)
		current_exposure = value;
	return ret;
}

int Eye::set_gain(unsigned int value)
{
	struct v4l2_control ctl;
	int ret;

        ctl.id = V4L2_CID_GAIN;
        ctl.value = value;
        ret = xioctl(this->fd, VIDIOC_S_CTRL, &ctl);
	if(ret != -1)
		current_gain = value;
	return ret;
}

int Eye::set_autogain(unsigned int enable)
{
	struct v4l2_control ctl;
	int ret;

        ctl.id = V4L2_CID_AUTOGAIN;
        ctl.value = enable? 1 : 0;
        ret = xioctl(this->fd, VIDIOC_S_CTRL, &ctl);
	if(ret != -1)
		current_autogain = ctl.value;
	return ret;
}

unsigned int Eye::get_exposure()
{
	return current_exposure;
}

unsigned int Eye::get_gain()
{
	return current_gain;
}

unsigned int Eye::get_autogain()
{
	return current_autogain;
}

void Eye::exec_line(ifstream &input)
{
	char cmd;
	unsigned int value;

	input >> cmd;
	switch(cmd) {
		case 'e':
			input >> value;
			set_exposure(value);
			break;
		case 'g':
			input >> value;
			set_gain(value);
			break;
		case 'a':
			input >> value;
			set_autogain(value);
			break;
		case 's':
			if(!can_save) {
				cout << "Can't save from config file" << endl;
				break;
			}
			save_config(EYE_CONFIG_FILENAME);
			break;
		default:
			if(can_save) {
				cout << "Help:" << endl;
				cout << "\te <0-100>: set exposure" << endl;
				cout << "\tg <0-100>: set gain" << endl;
				cout << "\ta <0-1>: set autogain" << endl;
			}
	}
}

void Eye::save_config(const char *filename)
{
	ofstream config_file;

	config_file.open(filename, ios_base::out|ios_base::trunc);
        config_file << "e " << current_exposure << endl;
        config_file << "g " << current_gain << endl;
        config_file << "a " << current_autogain << endl;
        config_file.close();
}

int Eye::cam_open(const char *eye_path)
{
	ifstream config_file;
	this->fd = open(eye_path, O_RDWR|O_NONBLOCK);
	if(this->fd == -1) {
		perror("open");
		return -1;
	}

	if(init() == -1) {
		this->cam_close();
		return -1;
	}

	can_save = false;
        config_file.open(EYE_CONFIG_FILENAME, ifstream::in);
        if(config_file.is_open()) {
                while(!config_file.eof())
                        exec_line(config_file);

                config_file.close();
        } else {
                set_exposure(100);
                set_gain(35);
                set_autogain(1);
                save_config(EYE_CONFIG_FILENAME);
        }
	can_save = true;
	return 0;
}

int Eye::stream_on()
{
	enum v4l2_buf_type type;
	struct v4l2_buffer buf;
	unsigned int i;

	for (i = 0; i < this->yuyv_bufs_count; i++) {
		memset(&buf, 0, sizeof(buf));

		// http://v4l2spec.bytesex.org/spec/r12878.htm
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if(xioctl(this->fd, VIDIOC_QBUF, &buf) == -1) {
			perror("ioctl");
			return -1;
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (xioctl(this->fd, VIDIOC_STREAMON, &type) == -1) {
		perror("ioctl");
		return -1;
	}

	return 0;
}

int Eye::stream_off()
{
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(xioctl(this->fd, VIDIOC_STREAMOFF, &type) == -1) {
		perror("ioctl");
		return -1;
	}

	return 0;
}

int Eye::frame_get(void (*func_process_image)(unsigned char *, void *), void *data)
{
	struct v4l2_buffer buf;

	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if(xioctl(this->fd, VIDIOC_DQBUF, &buf) == -1) {
		perror("ioctl");
		return -1;
	}

	v4lconvert_yuyv_to_rgb24((const unsigned char *)this->yuyv_bufs[buf.index].start, this->rgb24_buf, IMAGE_WIDTH, IMAGE_HEIGHT);

	if(func_process_image)
		func_process_image(this->rgb24_buf, data);

	if(xioctl(this->fd, VIDIOC_QBUF, &buf) == -1) {
		perror("ioctl");
		return -1;
	}

	return 0;
}

int Eye::frame_wait_for(struct timeval *tv)
{
	fd_set fds;
	int r;

	FD_ZERO (&fds);
	FD_SET (this->fd, &fds);

	do {
		r = select (this->fd + 1, &fds, NULL, NULL, tv);
	} while(r == -1 && errno == EINTR);

	return r == 0? -1 : 0;
}

void Eye::cam_close()
{
	unsigned int i;

	this->stream_off();

	for(i = 0; i < this->yuyv_bufs_count; i++)
		munmap(this->yuyv_bufs[i].start, this->yuyv_bufs[i].length);

	close(this->fd);
}

Eye::~Eye()
{
	this->cam_close();
}

#ifdef UNIT_TEST
void test_func(unsigned char *frame, void *data)
{
	printf("%u\n", frame[0]);
	//int fd = open("./raw_rgb", O_RDWR|O_CREAT, 0665);
	//write(fd, frame, 320*240*3);
	//close(fd);
	//exit(0);
}

int main()
{
	Eye eye;
	unsigned int count=0, timestamp;

	eye.cam_open("/dev/video0");
	eye.stream_on();

	timestamp = time(NULL);
	while(1) {
                        struct timeval tv;

			if(time(NULL) - timestamp >= 1) {
				printf("count=%u\n", count);
				count = 0;
				timestamp = time(NULL);
			}
                        /* Timeout. */
                        tv.tv_sec = 2;
                        tv.tv_usec = 0;
			eye.frame_wait_for(&tv);
			eye.frame_get(test_func, NULL);
			count++;
	}
	return 0;
}

#endif
