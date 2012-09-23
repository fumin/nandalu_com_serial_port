#include "com_port.h"

struct com_port_t {
	HANDLE hComm;
};

void phex(char* buf, int len) {
	for(char* p = buf; p != buf + len; ++p){
		printf("%02X ", (unsigned char)*p);
	}
	printf("\n");
}

void ErrorExit(LPTSTR lpszFunction) { 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}

void PrintCommState(DCB dcb) {
    //  Print some of the DCB structure values
    printf("\nBaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n", 
		dcb.BaudRate, 
		dcb.ByteSize, 
		dcb.Parity,
		dcb.StopBits );
}

int init_com_port(LPTSTR gszPort, HANDLE* phComm){
	*phComm = CreateFile( gszPort,
		GENERIC_READ | GENERIC_WRITE, 
        0,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0);
	if (*phComm == INVALID_HANDLE_VALUE) return -1;
      // error opening port; abort

	DCB dcb;
	//  Initialize the DCB structure.
    SecureZeroMemory(&dcb, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);
	//  Fill in some DCB values and set the com state: 
    //  57,600 bps, 8 data bits, no parity, and 1 stop bit.
    dcb.BaudRate = CBR_115200;     //  baud rate
    dcb.ByteSize = 8;             //  data size, xmit and rcv
    dcb.Parity   = NOPARITY;      //  parity bit
    dcb.StopBits = ONESTOPBIT;    //  stop bit
	BOOL fSuccess;
    if (!(fSuccess = SetCommState(*phComm, &dcb))) {
		//  Handle the error.
        printf ("SetCommState failed with error %d.\n", GetLastError());
		getchar();
        return (3);
    }
	PrintCommState(dcb);       //  Output to console

	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = 20; 
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.ReadTotalTimeoutConstant = 100;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 100;
    if (!SetCommTimeouts(*phComm, &timeouts)) return -1;
    // Error setting time-outs.
	return 0;
}

ComPort newComPort(int port_num) {
	ComPort cp;
	char port_buf[32];
	if (port_num<10) {
	    sprintf(port_buf, "COM%d", port_num);
		cp = newComPort(port_buf);	
	} else {
		sprintf(port_buf, "\\\\.\\COM%d", port_num);
		cp = newComPort(port_buf);
	}
	return cp;
}

ComPort newComPort(char* port) {
	ComPort com_port;
	com_port = (ComPort)malloc(sizeof(struct com_port_t));
	init_com_port(port, &com_port->hComm);
	return com_port;
}

void deleteComPort(ComPort com_port) {
	CloseHandle(com_port->hComm);
	free(com_port);
}

int read(ComPort cp, char* outbuf){
	char buf[512];
	DWORD numberOfBytesRead;
	int rc, msg_len, totalNumOfBytesRead;
	char* p = buf;
	totalNumOfBytesRead = 0;
	do {
		rc = ReadFile(cp->hComm, p, 512-(p-buf), &numberOfBytesRead, NULL);
		if(!rc) return -1;
		p += numberOfBytesRead;
		if(p-buf >= 512) return -1;
		totalNumOfBytesRead += numberOfBytesRead;

		for (char* pc = buf; pc != buf + totalNumOfBytesRead; pc++) {
			if (!memcmp(pc, "\xA5\x5A", 2)) {
				msg_len = (unsigned char)*(pc + 2);
				if ((pc-buf)+2+msg_len <= totalNumOfBytesRead) {
					// plus 3 because \xA5\x5A#{payload length}
					// minus 1 because the last byte is always \xBC
					memcpy(outbuf, pc+3, msg_len-1);
					outbuf[msg_len - 1] = '\0';
					return msg_len-1;
				} else {
					break;
				}
			}
		}
	} while (1);
}

void write(ComPort cp, char* buf, int len) {
	DWORD numberOfBytesWritten;
	WriteFile(cp->hComm, buf, len, &numberOfBytesWritten, NULL);
}

void write(ComPort cp, char* buf, int len, char* mac_addr) {
	char total_buf[256];
	memcpy(total_buf, "\xA5\x5A", 2);
	total_buf[2] = len + 8 + 1;
	memcpy(total_buf + 3, buf, len);
	memcpy(total_buf + 3 + len, mac_addr, 8);
	total_buf[3 + len + 8] = '\xBC';
	write(cp, total_buf, len + 8 + 4);
}

