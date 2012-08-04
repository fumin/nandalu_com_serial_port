#include "com_port.h"

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
    printf( TEXT("\nBaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n"), 
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