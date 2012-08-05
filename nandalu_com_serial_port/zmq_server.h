#ifndef _ZMQ_SERVER_H_
#define _ZMQ_SERVER_H_

#include "zmq.h"

typedef struct zmq_server_t* ZMQServer;

ZMQServer newZMQServer(void* context, char* addr, int type);
void deleteZMQServer(ZMQServer z);
int read(ZMQServer z, char* buf);
void write(ZMQServer z, char* buf);

#endif