#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdio.h>

void err_quit(char *msg) {
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

int main(int argc, char *argv[])
{
	// 윈속 초기화
	WSADATA wsa;
	// 하위 상위
	// 8비트씩 나눠서 읽어들인다.
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	MessageBox(NULL, "윈속 초기화 성공", "알림", MB_OK);
	printf("wVersion : %d\n", wsa.wVersion);
	printf("HIBYTE : %d\n", HIBYTE(wsa.wVersion));
	printf("LOBYTE : %d\n", LOBYTE(wsa.wVersion));
	

	printf("wHighVersion : %d\n", wsa.wHighVersion);
	printf("szDescription : %s\n", wsa.szDescription);
	printf("szSystemStatus : %s\n", wsa.szSystemStatus);


	SOCKET tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_sock == INVALID_SOCKET) err_quit("socket()");
	MessageBox(NULL, "TCP 소켓 생성 성공", "알림", MB_OK);

	closesocket(tcp_sock);


	// 윈속 종료
	WSACleanup();
	return 0;
}