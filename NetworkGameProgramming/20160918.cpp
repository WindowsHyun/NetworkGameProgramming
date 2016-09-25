//86Page - 3번 문제

#pragma comment (lib,"ws2_32")
#include <WinSock2.h>
#include <stdio.h>
#include <Windows.h>

// 소켓 함수 오류 출력
void err_display(char* msg) {
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void getIPv4Information(char* domain) {
	HOSTENT *ptr = gethostbyname(domain);
	//----------------------------------------------------------
	if (ptr == NULL) {
		err_display("gethostbyname()");
		exit(1);
	}
	if (ptr->h_addrtype != AF_INET)
		exit(1);
	//----------------------------------------------------------
	printf("\n");
	printf("도메인 이름 : %s\n", ptr->h_name);


	if (ptr->h_aliases[0] == NULL) {
		printf("별명 : NULL\n");
	}
	else {
		for (int i = 0; ptr->h_aliases[i] != NULL; ++i)
			printf("별명 %d : %s\n", i, ptr->h_aliases[i]);
	}

	for (int i = 0; ptr->h_addr_list[i] != NULL; ++i)
		printf("IP주소 %d : %s\n", i + 1, inet_ntoa(*(IN_ADDR*)ptr->h_addr_list[i]));
	//h_addr_list 안에 char형으로 주소를 담고 있어, IN_ADDR 구조체로 바꾼뒤 inet_ntoad로 s_addr을 추출


	printf("\n\n- nslookup 조회 - \n\n");
	char search[256];;
	sprintf(search, "nslookup %s", ptr->h_name);
	system(search);
}

int main(int argc, char* argv[]) {

	WSADATA wsa;
	char domainName[256] = {};
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	printf("도메인 이름을 입력하세요. \n->");
	scanf("%s", domainName);

	getIPv4Information(domainName);


	WSACleanup();

	return 0;
}









/*
h_addr_list 안에 char형으로 주소를 담고 있어서, IN_ADDR 구조체로 바꾼뒤 inet_ntoad로 s_addr을 추출하기 위해서
*/