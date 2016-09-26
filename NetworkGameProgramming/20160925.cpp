//130Page - 8번 문제
#pragma comment (lib,"ws2_32")
#include <WinSock2.h>
#include <stdio.h>


void getPortOnOff( char* domain, int sNum, int eNum ) {
	int retval;

	for ( int portNum = sNum; portNum <= eNum; ++portNum ) {
		// socket()
		SOCKET sock = socket( AF_INET, SOCK_STREAM, 0 );
		if ( sock == INVALID_SOCKET ) {
			printf( "서버와 연결을 할 수 없습니다..!" );
			exit( 1 );
		}

		// connect()
		SOCKADDR_IN serveraddr;
		ZeroMemory( &serveraddr, sizeof( serveraddr ) );
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = inet_addr( domain );
		serveraddr.sin_port = htons( portNum );
		retval = connect( sock, (SOCKADDR *)&serveraddr, sizeof( serveraddr ) );
		if ( retval == SOCKET_ERROR ) {
			printf( "[ %s:%d ] = Disconnected\n", domain, portNum );
		}
		else {
			printf( "[ %s:%d ] = Connected\n", domain, portNum );

		}
		closesocket( sock );
	}

}


int main( int argc, char* argv[] ) {
	int num, sNum, eNum;
	WSADATA wsa;
	HOSTENT *ptr;
	char domainName[512] = {};
	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != 0 )
		return 1;

	printf( "1. IP 주소\n2. 도메인 주소\n->" );
	scanf( "%d", &num );

	switch ( num ) {
	case 1:
		printf( "아이피 주소를 입력하세요. \n->" );
		scanf( "%s", domainName );
		break;
	case 2:
		printf( "도메인 이름을 입력하세요. \n->" );
		scanf( "%s", domainName );
		ptr = gethostbyname( domainName );
		memcpy( domainName, inet_ntoa( *(IN_ADDR*)ptr->h_addr_list[0] ), strlen( inet_ntoa( *(IN_ADDR*)ptr->h_addr_list[0] ) ) );
		break;
	default:
		printf( "제대로된 번호를 입력해주세요." );
		break;
	}

	printf( "포트번호를 적어주세요 ex)80-81\n->" );
	scanf( "%d-%d", &sNum, &eNum );
	printf( "\n" );


	getPortOnOff( domainName, sNum, eNum );

	WSACleanup();

	return 0;
}









/*
h_addr_list 안에 char형으로 주소를 담고 있어서, IN_ADDR 구조체로 바꾼뒤 inet_ntoad로 s_addr을 추출하기 위해서
*/