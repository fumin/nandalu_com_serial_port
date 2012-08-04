#ifndef _COM_PORT_H_
#define _COM_PORT_H_

#include <windows.h>
#include <strsafe.h>
#include <stdio.h>

void ErrorExit(LPTSTR lpszFunction);
void PrintCommState(DCB dcb);
int init_com_port(LPTSTR port, HANDLE* phComm);

#endif