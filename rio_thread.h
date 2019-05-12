#ifndef __RIO_THREAD_H__
#define __RIO_THREAD_H__

#include <pthread.h>
//void load_publish_file(char *path);
//void load_subscribe_file(char *path);
//void publoisher_init();

pthread_t *publish_task_init();
void publish_task_wait(pthread_t *tid);


pthread_t publisher_scanner_init();
void publisher_scanner_wait(pthread_t tid);

#endif