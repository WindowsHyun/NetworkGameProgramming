//86Page - 3�� ����

#pragma comment (lib,"ws2_32")
#include <WinSock2.h>
#include <stdio.h>
#include <Windows.h>

// ���� �Լ� ���� ���
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
	printf("������ �̸� : %s\n", ptr->h_name);


	if (ptr->h_aliases[0] == NULL) {
		printf("���� : NULL\n");
	}
	else {
		for (int i = 0; ptr->h_aliases[i] != NULL; ++i)
			printf("���� %d : %s\n", i, ptr->h_aliases[i]);
	}

	for (int i = 0; ptr->h_addr_list[i] != NULL; ++i)
		printf("IP�ּ� %d : %s\n", i + 1, inet_ntoa(*(IN_ADDR*)ptr->h_addr_list[i]));
	//h_addr_list �ȿ� char������ �ּҸ� ��� �־�, IN_ADDR ����ü�� �ٲ۵� inet_ntoad�� s_addr�� ����


	printf("\n\n- nslookup ��ȸ - \n\n");
	char search[256];;
	sprintf(search, "nslookup %s", ptr->h_name);
	system(search);
}

int main(int argc, char* argv[]) {

	WSADATA wsa;
	char domainName[256] = {};
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	printf("������ �̸��� �Է��ϼ���. \n->");
	scanf("%s", domainName);

	getIPv4Information(domainName);


	WSACleanup();

	return 0;
}









/*
h_addr_list �ȿ� char������ �ּҸ� ��� �־, IN_ADDR ����ü�� �ٲ۵� inet_ntoad�� s_addr�� �����ϱ� ���ؼ�
*/