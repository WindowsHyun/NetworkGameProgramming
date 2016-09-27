#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <atlstr.h> // cstring
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    3000

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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

// 사용자 정의 데이터 수신 함수
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

int main( int argc, char *argv[] )
{
	int retval;

	// 윈속 초기화
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

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	int len;
	char filename[BUFSIZE];

	while ( 1 ) {
		// accept()
		addrlen = sizeof( clientaddr );
		client_sock = accept( listen_sock, (SOCKADDR *)&clientaddr, &addrlen );
		if ( client_sock == INVALID_SOCKET ) {
			err_display( "accept()" );
			break;
		}

		// 접속한 클라이언트 정보 출력
		printf( "\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa( clientaddr.sin_addr ), ntohs( clientaddr.sin_port ) );
		// 클라이언트와 데이터 통신
		while ( 1 ) {
			//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
			// 파일명 받아오기

			retval = recvn( client_sock, (char *)&len, sizeof( int ), 0 ); // 데이터 받기(고정 길이)
			if ( retval == SOCKET_ERROR ) {
				err_display( "recv()" );
				break;
			}
			else if ( retval == 0 )
				break;

			char *buf = new char[len]; // 전송된 길이를 알고 있으니 크기에 맞춰서 buf를 늘려주자!

			// 데이터 받기(가변 길이)
			retval = recvn( client_sock, buf, len, 0 );
			if ( retval == SOCKET_ERROR ) {
				err_display( "recv()" );
				break;
			}
			else if ( retval == 0 )
				break;

			// 받은 데이터 출력
			buf[retval] = '\0';
			sprintf( filename, "%s", buf );
			printf( "파일 이름 : %s [%d byte]\n", filename, len );
			CString str = filename;
			CString fileexe = str.Right(str.GetLength() - str.ReverseFind('\\') - 1);
			printf("파일 명 : %s\n", fileexe);

			//delete[] buf;
			//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
			// 실제 파일 데이터를 받기

			retval = recvn( client_sock, (char *)&len, sizeof( int ), 0 );
			if ( retval == SOCKET_ERROR ) {
				err_display( "recv()" );
				break;
			}
			else if ( retval == 0 )
				break;

			char *buffer = new char[len]; // 전송된 길이를 알고 있으니 크기에 맞춰서 buffer를 늘려주자!

			retval = recvn( client_sock, buffer, len, 0 );
			if ( retval == SOCKET_ERROR ) {
				err_display( "recv()" );
				break;
			}
			else if ( retval == 0 )
				break;

			// 받은 데이터 출력
			buffer[retval] = '\0';
			FILE *fp;
			fp = fopen(fileexe, "wb" );
			fwrite( buffer, 1, len, fp );
			fclose( fp );
			printf( "데이터 길이 : %d byte\n", len );


		}
		closesocket( client_sock );
		//printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// closesocket()
	closesocket( listen_sock );

	// 윈속 종료
	WSACleanup();
	return 0;
}