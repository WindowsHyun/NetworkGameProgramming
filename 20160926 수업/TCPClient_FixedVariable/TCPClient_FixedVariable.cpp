#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <fstream>

using namespace std;

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    3000

// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ������ ��ſ� ����� ����
	FILE *fp;
	char fileName[MAX_PATH]; // ������ �̸�
	int fileSize;
	while ( 1 ) {

		while ( true ) {
			printf( "���� �̸��� �Է��ϼ���\n->" );
			scanf( "%s", &fileName );
			if ( (fp = fopen( fileName, "rb" )) == NULL ) {
				fputs( "���� ���� ����!\nerror : ", stderr );
				continue;
			}
			else {
				break;
			}
		}

		fseek( fp, 0L, SEEK_END ); //���� ������ ��ġ �ű�
		fileSize = ftell( fp ); //���� ����Ʈ�� ���
		fseek( fp, 0L, SEEK_SET ); //�ٽ� ���� ó������ ��ġ �ű�
		printf( "-----------------------------------------------\n" );
		printf( "������ ũ�� : %d bytes \n", fileSize );
		printf( "������ �̸� : %s\n", fileName );
		printf( "-----------------------------------------------\n" );
		//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		int len = strlen( fileName );
		char *namebuf = new char[len];
		strncpy( namebuf, fileName, len );
		// ������ ������(���� ����)
		retval = send( sock, (char *)&len, sizeof( int ), 0 );
		if ( retval == SOCKET_ERROR ) {
			err_display( "send()" );
			exit( 1 );
		}
		// ������ ������(���� ����)
		retval = send( sock, namebuf, len, 0 );
		if ( retval == SOCKET_ERROR ) {
			err_display( "send()" );
			exit( 1 );
		}
		printf( "send filename size : %d byte\n", retval );

		//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		char *buf = new char[fileSize];

		fread( buf, 1, fileSize, fp );

		retval = send( sock, (char *)&fileSize, sizeof( int ), 0 );
		if ( retval == SOCKET_ERROR ) {
			err_display( "send()" );
			exit( 1 );
		}
		// ������ ������(���� ����)
		retval = send( sock, buf, fileSize, 0 );
		if ( retval == SOCKET_ERROR ) {
			err_display( "send()" );
			exit( 1 );
		}
		printf( "send file size : %d byte\n\n", retval );
		//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	}

	closesocket(sock);
	
	// ���� ����
	WSACleanup();
	return 0;
}