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
