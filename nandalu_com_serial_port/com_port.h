#ifndef _COM_PORT_H_
#define _COM_PORT_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <strsafe.h>
#include <stdlib.h>

typedef struct com_port_t* ComPort;

ComPort newComPort(char* port);
void deleteComPort(ComPort com_port);
int read(ComPort cp, char* buf);
void write(ComPort cp, char* buf);

void ErrorExit(LPTSTR lpszFunction);
void phex(char* buf, int len);

#endif