#include "com_port.h"
#include "zmq_server.h"

#define BUFFER_SIZE 512

int main() {
	int port_num = 0;
    printf("plz enter the COM Port number:\n");
    scanf("%d",&port_num);
	ComPort cp = newComPort(port_num);

	void* context = zmq_ctx_new();
	ZMQServer zmq_server = newZMQServer(context, "tcp://*:5555", ZMQ_REP);
	ZMQServer zmq_publisher = newZMQServer(context, "tcp://*:5556", ZMQ_PUB);
	
    char buf[BUFFER_SIZE];
	int len;
	while(1){
		char port_cmd[32];
		len = read(zmq_server, buf);
		if (!strcmp("lights on", buf + 9)) {
			write(cp, "\xAA", 1, buf);
		} else if (!strcmp("lights off", buf + 9)) {
			write(cp, "\xBB", 1, buf);
		}

		len = read(cp, buf);
		phex(buf, len);
		char light_ack[32];
		if (!strncmp("\xAB\xCD", buf, 2)) {
			memcpy(light_ack, buf + 2, 8);
			memcpy(light_ack + 8, " lights on OK", 13);
			printf("light_ack = %s\n", light_ack);
			write(zmq_server, light_ack, 21);
		} else if (!strncmp("\xCD\xEF", buf, 2)) {
			memcpy(light_ack, buf + 2, 8);
			memcpy(light_ack + 8, " lights off OK", 14);
			printf("light_ack = %s\n", light_ack);
			write(zmq_server, light_ack, 22);
		} else if (!strncmp("\x00\x00", buf, 2)) {
			memcpy(light_ack, buf + 2, 8);
			memcpy(light_ack + 8, " lights error", 13);
			printf("light_ack = %s\n", light_ack);
			write(zmq_server, light_ack, 21);
		} else {
			write(zmq_publisher, (char*)buf, len);
		}
	}
}

