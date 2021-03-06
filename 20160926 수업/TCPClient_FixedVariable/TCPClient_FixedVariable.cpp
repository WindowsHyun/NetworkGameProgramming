#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <Windows.h>
#include <stdio.h>



#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define DataBufSize    1024

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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

	// 윈속 초기화
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
	// 데이터 통신에 사용할 변수
	FILE *fp;
	char fileName[MAX_PATH]; // 파일의 이름
	int fileSize;
	int sendSize;
	float startTime, endTime;

	while ( 1 ) {
		while ( true ) {
			printf( "파일 이름을 입력하세요\n->" );
			scanf( "%s", &fileName );
			if ( (fp = fopen( fileName, "rb" )) == NULL ) {
				fputs( "파일 열기 에러!\nerror : ", stderr );
				continue;
			}
			else {
				break;
			}
		}

		fseek( fp, 0L, SEEK_END ); //파일 끝으로 위치 옮김
		fileSize = ftell( fp ); //파일 바이트값 출력
		fseek( fp, 0L, SEEK_SET ); //다시 파일 처음으로 위치 옮김
		printf( "-----------------------------------------------\n" );
		printf( "데이터 크기 : %d bytes \n", fileSize );
		printf( "데이터 이름 : %s\n", fileName );
		printf( "-----------------------------------------------\n" );
		//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		int len = strlen( fileName );
		char *namebuf = new char[len];
		strncpy( namebuf, fileName, len );
		// 데이터 보내기(고정 길이)
		retval = send( sock, (char *)&len, sizeof( int ), 0 );
		if ( retval == SOCKET_ERROR ) {
			err_display( "send()" );
			exit( 1 );
		}
		// 데이터 보내기(가변 길이)
		retval = send( sock, namebuf, len, 0 );
		if ( retval == SOCKET_ERROR ) {
			err_display( "send()" );
			exit( 1 );
		}
		//printf( "send filename size : %d byte\n", retval );

		//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		char buf[DataBufSize];
		int bufSize = DataBufSize;
		int nowsendSize = 0;
		float nowPer;
		int percentage = 10;
		bool sendData = true;
		sendSize = fileSize;

		retval = send( sock, (char *)&fileSize, sizeof( int ), 0 );
		if ( retval == SOCKET_ERROR ) {
			err_display( "send()" );
			exit( 1 );
		}
		printf( "진행도 ( 10%% 당 1개씩 ) : [ " );
		
		startTime = GetTickCount();
		while ( sendData ) {
			//파일 읽어서 버퍼에 저장
			if ( sendSize <= DataBufSize ) {
				bufSize = DataBufSize - sendSize;
				sendData = false;
			}
			else {
				bufSize = DataBufSize;
			}

			fread( buf, 1, bufSize, fp );

			//전송
			retval = send( sock, buf, bufSize, 0 );
			if ( retval == SOCKET_ERROR ) {
				err_display( "send()" );
				exit( 1 );
			}
			sendSize -= bufSize;
			nowsendSize += bufSize;
			nowPer = (float)nowsendSize / (float)fileSize * 100;

			if ( nowPer >= percentage ) {
				percentage += 10;
				printf( "★" );
			}
		}
		endTime = GetTickCount();
		printf( " ]\n" );
		printf( "총 소요시간 : %f초\n", (endTime-startTime)/1024 );
		//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	}

	closesocket(sock);
	
	// 윈속 종료
	WSACleanup();
	return 0;
}