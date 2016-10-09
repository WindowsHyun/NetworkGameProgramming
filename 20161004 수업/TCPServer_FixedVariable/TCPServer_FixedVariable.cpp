#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <atlstr.h> // cstring
#include <Windows.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    3000
#define DataBufSize    1024

// ���� �Լ� ���� ��� �� ����
void err_quit( char *msg )
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		(LPTSTR)&lpMsgBuf, 0, NULL );
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR );
	LocalFree( lpMsgBuf );
	exit( 1 );
}

// ���� �Լ� ���� ���
void err_display( char *msg )
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		(LPTSTR)&lpMsgBuf, 0, NULL );
	printf( "[%s] %s", msg, (char *)lpMsgBuf );
	LocalFree( lpMsgBuf );
}

// ����� ���� ������ ���� �Լ�
int recvn( SOCKET s, char *buf, int len, int flags )
{
	int received;
	char *ptr = buf;
	int left = len;

	while ( left > 0 ) {
		received = recv( s, ptr, left, flags );
		if ( received == SOCKET_ERROR )
			return SOCKET_ERROR;
		else if ( received == 0 )
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}


DWORD WINAPI ProcessClient( LPVOID arg ) {
	SOCKET client_sock = (SOCKET)arg;
	SOCKADDR_IN clientaddr;
	int addrlen;
	int retval;
	int len;
	int reciveSize; //���� ������� ������ ���ؼ� ���� ��ŭ ���������� Ȯ���� ���ؼ�
	int nowreciveSize;
	int percentage;
	int fileSize;
	float nowPer;
	float startTime, endTime;
	char filename[BUFSIZE];

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof( clientaddr );
	getpeername( client_sock, (SOCKADDR *)&clientaddr, &addrlen );

	while ( 1 ) {
		//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// ���ϸ� �޾ƿ���

		retval = recvn( client_sock, (char *)&len, sizeof( int ), 0 ); // ������ �ޱ�(���� ����)
		if ( retval == SOCKET_ERROR ) {
			err_display( "recv()" );
			break;
		}
		else if ( retval == 0 )
			break;

		char *buf = new char[len]; // ���۵� ���̸� �˰� ������ ũ�⿡ ���缭 buf�� �÷�����!

								   // ������ �ޱ�(���� ����)
		retval = recvn( client_sock, buf, len, 0 );
		if ( retval == SOCKET_ERROR ) {
			err_display( "recv()" );
			break;
		}
		else if ( retval == 0 )
			break;

		// ���� ������ ���
		buf[retval] = '\0';
		sprintf( filename, "%s", buf );

		CString str = filename;
		CString fileexe = str.Right( str.GetLength() - str.ReverseFind( '\\' ) - 1 );


		//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// ���� ���� �����͸� �ޱ�
		retval = recvn( client_sock, (char *)&reciveSize, sizeof( int ), 0 );
		if ( retval == SOCKET_ERROR ) {
			err_display( "recv()" );
			break;
		}
		else if ( retval == 0 )
			break;

		fileSize = reciveSize;
		percentage = 10;
		nowreciveSize = 0;


		printf( "-----------------------------------------------\n" );
		printf( "Client IP : %s, Port : %d\n", inet_ntoa( clientaddr.sin_addr ), ntohs( clientaddr.sin_port ) );
		printf( "���� �� : %s [%d byte]\n", fileexe, reciveSize );
		printf( "�ۼ������� : 0%%\n" );
		printf( "-----------------------------------------------\n" );

		char buffer[DataBufSize]; // ���۵� ���̸� �˰� ������ ũ�⿡ ���缭 buffer�� �÷�����!
		int bufSize = DataBufSize;
		bool sendData = true;

		FILE *fp;
		fp = fopen( fileexe, "wb" );
		startTime = GetTickCount();
		while ( sendData ) {
			if ( reciveSize <= DataBufSize ) {
				bufSize = DataBufSize - reciveSize;
				sendData = false;
			}
			else {
				bufSize = DataBufSize;
			}


			retval = recvn( client_sock, buffer, bufSize, 0 );
			if ( retval == SOCKET_ERROR ) {
				err_display( "recv()" );
				break;
			}
			else if ( retval == 0 )
				break;


			fwrite( buffer, 1, bufSize, fp );
			reciveSize -= bufSize;
			nowreciveSize += bufSize;
			nowPer = (float)nowreciveSize / (float)fileSize * 100;

			if ( nowPer >= percentage ) {
				percentage += 10;
				printf( "-----------------------------------------------\n" );
				printf( "Client IP : %s, Port : %d\n", inet_ntoa( clientaddr.sin_addr ), ntohs( clientaddr.sin_port ) );
				printf( "���� �� : %s\n", fileexe );
				printf( "�ۼ������� : %f%%\n", nowPer );
				printf( "-----------------------------------------------\n" );
			}

		}
		endTime = GetTickCount();

		printf( "-----------------------------------------------\n" );
		printf( "Client IP : %s, Port : %d\n", inet_ntoa( clientaddr.sin_addr ), ntohs( clientaddr.sin_port ) );
		printf( "���� �� : %s\n", fileexe );
		printf( "�ۼ������� : 100%%\n" );
		printf( "�� �ҿ�ð� : %f��\n->Download the complete file!\n", (endTime - startTime) / 1024 );
		printf( "-----------------------------------------------\n" );
		fclose( fp );

	}
	closesocket( client_sock );

	return 0;
}

int main( int argc, char *argv[] )
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != 0 )
		return 1;

	// socket()
	SOCKET listen_sock = socket( AF_INET, SOCK_STREAM, 0 );
	if ( listen_sock == INVALID_SOCKET ) err_quit( "socket()" );

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory( &serveraddr, sizeof( serveraddr ) );
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl( INADDR_ANY );
	serveraddr.sin_port = htons( SERVERPORT );
	retval = bind( listen_sock, (SOCKADDR *)&serveraddr, sizeof( serveraddr ) );
	if ( retval == SOCKET_ERROR ) err_quit( "bind()" );

	// listen()
	retval = listen( listen_sock, SOMAXCONN );
	if ( retval == SOCKET_ERROR ) err_quit( "listen()" );

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	HANDLE hThread;

	while ( 1 ) {
		// accept()
		addrlen = sizeof( clientaddr );
		client_sock = accept( listen_sock, (SOCKADDR *)&clientaddr, &addrlen );
		if ( client_sock == INVALID_SOCKET ) {
			err_display( "accept()" );
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		printf( "\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa( clientaddr.sin_addr ), ntohs( clientaddr.sin_port ) );

		// ������ ����
		hThread = CreateThread( NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL );
		if ( hThread == NULL ) {
			closesocket( client_sock );
		}
		else {
			CloseHandle( hThread );
		}

	}
	// closesocket()
	closesocket( listen_sock );

	// ���� ����
	WSACleanup();
	return 0;
}