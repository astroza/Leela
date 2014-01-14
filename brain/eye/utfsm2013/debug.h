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

#ifndef DEBUG_H
#define DEBUG_H

#include "config.h"
#include <semaphore.h>
#include <fstream>

using namespace std;

#define IMAGE_SENDER_CHUNK_SIZE 25*1024

class Debug_interp {
	public:
		virtual void exec_line(ifstream &input) = 0;
};

class Debug {
	static Debug *instance;
	private:
		sem_t image_sender_wait;
		sem_t buffer_sender_lock;
		int client_fd;
		int server_fd;
		unsigned char sender_buffer[IMAGE_SIZE];
		void client_wait_for();
		void server_init(const char *addr, unsigned short port);
	public:
		Debug(Debug_interp *intep);
		static bool is_enabled();
		void image_send();
		void image_try_to_feed(void *image, int size);
		Debug();
};

#endif
