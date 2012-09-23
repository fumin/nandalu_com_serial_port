#include "zmq_server.h"

struct zmq_server_t {
	void* socket;
};

ZMQServer newZMQServer(void* context, char* addr, int type){
	ZMQServer z = (ZMQServer)malloc(sizeof(struct zmq_server_t));
	z->socket = zmq_socket (context, type);
	zmq_bind (z->socket, addr);
	return z;
};

void deleteZMQServer(ZMQServer z){
	zmq_close(z->socket);
	free(z);
};

int read(ZMQServer z, char* buf){
	zmq_pollitem_t items [1];
	items[0].socket = z->socket;
	items[0].events = ZMQ_POLLIN;
	int rc = zmq_poll(items, 1, 0); // 0 timeout
	if (rc == 1) {
		int msg_size;
		zmq_msg_t msg;
		zmq_msg_init (&msg);
		zmq_msg_recv (&msg, items[0].socket, 0);
		msg_size = zmq_msg_size(&msg);
		memcpy(buf, zmq_msg_data(&msg), msg_size);
		buf[msg_size] = '\0';
		zmq_msg_close(&msg);
		return msg_size;
	}
	buf[0] = '\0';
	return 0;
};

void write(ZMQServer z, char* buf, int len){
	zmq_send (z->socket, buf, len, 0);
};
