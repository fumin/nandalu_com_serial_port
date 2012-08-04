#include "com_port.h"

#define BUFFER_SIZE 256

int main() {
	HANDLE hComm;
	init_com_port(TEXT("COM3"), &hComm);
	
	TBYTE lpBuffer[BUFFER_SIZE];
	DWORD numberOfBytesRead, numberOfBytesWritten;

	while(ReadFile(hComm, lpBuffer, BUFFER_SIZE, &numberOfBytesRead, NULL)){
		// do our work
		for (int i = 0; i < numberOfBytesRead; i++) {
			if (i > 0) printf(" ");
			printf("%02X", lpBuffer[i]);
		}
		printf("\n");

		// notify when our light is dim
		if (lpBuffer[1] < 16) {
			WriteFile(hComm, "\xA5\x5A\x03\x11\x22\xBC", 6, &numberOfBytesWritten, NULL);
		}
	}

	ErrorExit(TEXT("ReadFile"));
}

