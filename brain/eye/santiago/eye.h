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

#ifndef EYE_H
#define EYE_H

#include <time.h>
#include "config.h"
#include <fstream>
#include "debug.h"

using namespace std;

#define EYE_CONFIG_FILENAME	"./eye.config"
class Eye : public Debug_interp {
	public:
		int cam_open(const char *);
		int stream_on();
		int stream_off();
		int frame_get(void (*)(unsigned char *, void *), void *);
		int frame_wait_for(struct timeval *tv);
		void cam_close();
		int set_exposure(unsigned int value);
		int set_gain(unsigned int value);
		int set_autogain(unsigned int value);
		void save_config(const char *filename);
		void exec_line(ifstream &input);
		unsigned int get_exposure();
		unsigned int get_gain();
		unsigned int get_autogain();
		unsigned char rgb24_buf[320*240*3];
		~Eye();
	private:
		int init();
		int fd;
		unsigned int current_exposure;
		unsigned int current_gain;
		unsigned int current_autogain;
		bool can_save;
		struct {
			void *start;
			unsigned long length;
		} yuyv_bufs[BUFFERS_COUNT];
		unsigned int yuyv_bufs_count;
};

#endif
