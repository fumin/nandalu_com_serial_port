#include <mbstring.h>

#include "com_port.h"
#include "zmq_server.h"

#define BUFFER_SIZE 512

int main() {
	void* context = zmq_ctx_new();
	ZMQServer zmq_server = newZMQServer(context, "tcp://*:5555", ZMQ_REP);
	ZMQServer zmq_publisher = newZMQServer(context, "tcp://*:5556", ZMQ_PUB);

	ComPort cp = newComPort("COM5");
    char buf[BUFFER_SIZE];
	int len;
	while(1){
		len = read(zmq_server, buf);
		if (!strcmp("lights on", buf)) {
			write(cp, "\xA5\x5A\x02\xAA\xBC");
		} else if (!strcmp("lights off", buf)) {
			write(cp, "\xA5\x5A\x02\xBB\xBC");
		}

		len = read(cp, buf);
		phex(buf, len);
		if (!strcmp("\xAB\xCD", buf)) {
			printf("Lights ON! send to web server!\n");
			write(zmq_server, "lights on OK");
		} else if (!strcmp("\xCD\xEF", buf)) {
			write(zmq_server, "lights off OK");
		} else if (!strcmp("\x00\x00", buf)) {
			write(zmq_server, "lights error");
		} else {
			write(zmq_publisher, (char*)buf);
		}
	}
}

