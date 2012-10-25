#include "subscriber.h"

struct subscriber_t {
	void* socket;
};

Subscriber newSubscriber(void* context, char* addr, zlist_t* device_ids) {
	Subscriber z = (Subscriber)malloc(sizeof(struct subscriber_t));
	z->socket = zmq_socket (context, ZMQ_SUB);
	zmq_connect (z->socket, addr);
	
	void* device_id = zlist_first(device_ids);
	do {
		zmq_setsockopt(z->socket, ZMQ_SUBSCRIBE, device_id, strlen((char*)device_id));
	} while(NULL != (device_id = zlist_next(device_ids)));

	return z;
};

void deleteSubscriber(Subscriber z) {
	zmq_close(z->socket);
	free(z);
};

int recv(Subscriber z, char* buf) {
	int msg_size;
	zmq_msg_t msg;
	zmq_msg_init (&msg);
	if (-1 == (msg_size = zmq_msg_recv (&msg, z->socket, 0))) {
		return -1;
	} else {
		memcpy(buf, zmq_msg_data(&msg), msg_size);
		zmq_msg_close(&msg);
		buf[msg_size] = '\0';
		return msg_size;
	}
}