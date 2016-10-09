#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <atlstr.h> // cstring
#include <Windows.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    3000
#define DataBufSize    1024

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


DWORD WINAPI ProcessClient( LPVOID arg ) {
	SOCKET client_sock = (SOCKET)arg;
	SOCKADDR_IN clientaddr;
	int addrlen;
	int retval;
	int len;
	int reciveSize; //버퍼 사이즈로 보내기 위해서 현재 얼만큼 보내졌는지 확인을 위해서
	int nowreciveSize;
	int percentage;
	int fileSize;
	float nowPer;
	float startTime, endTime;
	char filename[BUFSIZE];

	// 클라이언트 정보 얻기
	addrlen = sizeof( clientaddr );
	getpeername( client_sock, (SOCKADDR *)&clientaddr, &addrlen );

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

		CString str = filename;
		CString fileexe = str.Right( str.GetLength() - str.ReverseFind( '\\' ) - 1 );


		//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// 실제 파일 데이터를 받기
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
		printf( "파일 명 : %s [%d byte]\n", fileexe, reciveSize );
		printf( "퍼센테이지 : 0%%\n" );
		printf( "-----------------------------------------------\n" );

		char buffer[DataBufSize]; // 전송된 길이를 알고 있으니 크기에 맞춰서 buffer를 늘려주자!
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
				printf( "파일 명 : %s\n", fileexe );
				printf( "퍼센테이지 : %f%%\n", nowPer );
				printf( "-----------------------------------------------\n" );
			}

		}
		endTime = GetTickCount();

		printf( "-----------------------------------------------\n" );
		printf( "Client IP : %s, Port : %d\n", inet_ntoa( clientaddr.sin_addr ), ntohs( clientaddr.sin_port ) );
		printf( "파일 명 : %s\n", fileexe );
		printf( "퍼센테이지 : 100%%\n" );
		printf( "총 소요시간 : %f초\n->Download the complete file!\n", (endTime - startTime) / 1024 );
		printf( "-----------------------------------------------\n" );
		fclose( fp );

	}
	closesocket( client_sock );

	return 0;
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
	HANDLE hThread;

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

		// 스레드 생성
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

	// 윈속 종료
	WSACleanup();
	return 0;
}