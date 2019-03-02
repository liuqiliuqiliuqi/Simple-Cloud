#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<assert.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>

void thread_start(int c);

void* work_thread(void* arg);



