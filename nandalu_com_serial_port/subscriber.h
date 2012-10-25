#ifndef _SUBSCRIBER_H_
#define _SUBSCRIBER_H_

#include "zmq.h"
#include "zlist.h"

typedef struct subscriber_t* Subscriber;

Subscriber newSubscriber(void* context, char* addr, zlist_t* device_ids);
void deleteSubscriber(Subscriber z);
int recv(Subscriber z, char* buffer);

#endif