#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "debug.h"

#define EYE_STREAM

Debug *Debug::instance = NULL;

void *image_sender_thread(void *_debug)
{
	Debug *debug = (Debug *)_debug;

	do {
		debug->image_send();
	} while(1);

	return NULL; /* useless */
}

void *debug_interp_thread(void *_interp)
{
        Debug_interp *interp = (Debug_interp *)_interp;
	ifstream input;

	input.open("/dev/stdin");
        do {
		interp->exec_line(input);
        } while(1);

        return NULL; /* useless */
}

bool Debug::is_enabled()
{
	if(Debug::instance != NULL)
		return true;
	return false;
}

void Debug::image_send()
{
	int buffer_offset;
	int ret;

	sem_wait(&this->image_sender_wait); // Espera una imagen nueva
	sem_wait(&this->buffer_sender_lock); // Bloquea el buffer
	for(buffer_offset = 0; buffer_offset < IMAGE_SIZE; buffer_offset += IMAGE_SENDER_CHUNK_SIZE) {
		ret = write(this->client_fd, this->sender_buffer + buffer_offset, IMAGE_SENDER_CHUNK_SIZE);
		if(ret == -1) {
			perror("write");
			puts("Debug image sender error. Can't send buffer");
		}
	}
	sem_post(&this->buffer_sender_lock); // Desbloquea el buffer
}


void Debug::image_try_to_feed(void *image, int size = IMAGE_SIZE)
{
	if(sem_trywait(&this->buffer_sender_lock) != -1) {
		memcpy(this->sender_buffer, image, size);
		sem_post(&this->buffer_sender_lock);
		sem_post(&this->image_sender_wait);
	}
}

void Debug::server_init(const char *addr, unsigned short port)
{
	struct sockaddr_in sin;
	int fd, set = 1;

	printf("Starting server %s:%hu: ", addr, port);
	fflush(stdout);
	fd = socket(PF_INET, SOCK_STREAM, 0);

	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = inet_addr(addr);

	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set)) == -1) {
		perror("setsockopt");
		exit(-1);
	}

	if(bind(fd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) == -1) {
		close(fd);
		perror("bind");
		exit(-1);
	}
	puts("OK");
#ifdef EYE_STREAM
	listen(fd, 1);
#endif
	this->server_fd = fd;
}

void Debug::client_wait_for()
{
	struct sockaddr_in sin;
	socklen_t slen = sizeof(sin);
#ifndef EYE_STREAM
	char hello[5];
	do {   
		recvfrom(this->server_fd, hello, sizeof(hello), MSG_WAITALL, (struct sockaddr *)&sin, &slen);
	} while(strncmp(hello, "hello", 5) != 0);

	connect(this->server_fd, (struct sockaddr *)&sin, slen);
	this->client_fd = this->server_fd;
#else
	puts("Waiting a debug client");
	this->client_fd = accept(this->server_fd, (struct sockaddr *)&sin, &slen);
#endif	
}

Debug::Debug(Debug_interp *interp)
{
	server_init("0.0.0.0", 3334);
	client_wait_for();
	Debug::instance = this;
        sem_init(&this->image_sender_wait, 0, 0);
        sem_init(&this->buffer_sender_lock, 0, 1);
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, image_sender_thread, (void *)this);
	pthread_create(&tid, &attr, debug_interp_thread, (void *)interp);
}

