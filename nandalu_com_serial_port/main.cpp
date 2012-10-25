#include "com_port.h"
#include "zmq_server.h"
#include "subscriber.h"
#include <Windows.h>
#include <process.h>

#define BUFFER_SIZE 512

struct pass_command_thread_args_t {
	void* context;
	char* addr;
	zlist_t* device_ids;
	ComPort cp;
};

unsigned __stdcall pass_command_thread_func( void* pArguments ) {
    char buf[BUFFER_SIZE];
	int msg_size;
	struct pass_command_thread_args_t* args = (struct pass_command_thread_args_t*)pArguments;
    Subscriber subscriber = newSubscriber(args->context, args->addr, args->device_ids);
	printf( "In pass_command_thread_func...\n" );

	char just_for_testing[BUFFER_SIZE];
	just_for_testing[2] = '\0';

	while(1) {
		if (-1 == (msg_size = recv(subscriber, buf))) {
			printf("pass command thread error: %d, %s\nSleeping 5 seconds and reconnect\n", errno, strerror(errno));
			Sleep(5000);
			deleteSubscriber(subscriber);
			subscriber = newSubscriber(args->context, args->addr, args->device_ids);
			continue;
		}

		// do some processing
		if (!strcmp("turn_on", buf + 11)) {
			memcpy(just_for_testing, buf+8, 2);
			printf("pin number %d turn on!!!!!!!!!!!!!!!!!!!!!!", atoi(just_for_testing));
			write(args->cp, "\xAA", 1, buf);
		} else if (!strcmp("turn_off", buf + 11)) {
			memcpy(just_for_testing, buf+8, 2);
			printf("pin number %d turn off<<<<<<<<<<<<<<<<<<<", atoi(just_for_testing));
			write(args->cp, "\xBB", 1, buf);
		}
		phex(buf, 8);
	}
    return 0;
};

struct report_statuses_thread_args_t {
	void* context;
	char* addr;
	ComPort cp;
};

unsigned __stdcall report_statuses_thread_func(void* pArguments) {
	char buf[BUFFER_SIZE];
	int msg_size;
	struct report_statuses_thread_args_t* args = (struct report_statuses_thread_args_t*)pArguments;
	void* pusher = zmq_socket(args->context, ZMQ_PUSH);
	zmq_connect(pusher, args->addr);
	printf("In report_statuses_thread_func...\n");

	while(1) {
		// read statuses from com port
		msg_size = read(args->cp, buf);
		phex(buf, msg_size);

		zmq_send(pusher, buf, msg_size, ZMQ_DONTWAIT);
	}
	return 0;
};

int main() {
	int port_num;
    printf("plz enter the COM Port number:\n");
    scanf("%d",&port_num);
	ComPort cp = newComPort(port_num);
	void* context = zmq_ctx_new();

	zlist_t *device_ids = zlist_new();
	zlist_append(device_ids, ""); // subscribe to everything

	struct pass_command_thread_args_t pass_command_thread_args;
	pass_command_thread_args.context = context;
	pass_command_thread_args.addr = "tcp://140.113.240.134:5556";
	pass_command_thread_args.device_ids = device_ids;
	pass_command_thread_args.cp = cp;
	HANDLE pass_command_thread = (HANDLE)_beginthreadex(NULL, 0, &pass_command_thread_func, &pass_command_thread_args, 0, NULL);

	struct report_statuses_thread_args_t report_statuses_thread_args;
	report_statuses_thread_args.context = context;
	report_statuses_thread_args.addr = "tcp://140.113.240.134:5555";
	report_statuses_thread_args.cp = cp;
	HANDLE report_statuses_thread = (HANDLE)_beginthreadex(NULL, 0, &report_statuses_thread_func, &report_statuses_thread_args, 0, NULL);
	
	WaitForSingleObject( pass_command_thread, INFINITE );
	WaitForSingleObject( report_statuses_thread, INFINITE );
	zlist_destroy (&device_ids);
};

