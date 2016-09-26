//130Page - 8�� ����
#pragma comment (lib,"ws2_32")
#include <WinSock2.h>
#include <stdio.h>


void getPortOnOff( char* domain, int sNum, int eNum ) {
	int retval;

	for ( int portNum = sNum; portNum <= eNum; ++portNum ) {
		// socket()
		SOCKET sock = socket( AF_INET, SOCK_STREAM, 0 );
		if ( sock == INVALID_SOCKET ) {
			printf( "������ ������ �� �� �����ϴ�..!" );
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

	printf( "1. IP �ּ�\n2. ������ �ּ�\n->" );
	scanf( "%d", &num );

	switch ( num ) {
	case 1:
		printf( "������ �ּҸ� �Է��ϼ���. \n->" );
		scanf( "%s", domainName );
		break;
	case 2:
		printf( "������ �̸��� �Է��ϼ���. \n->" );
		scanf( "%s", domainName );
		ptr = gethostbyname( domainName );
		memcpy( domainName, inet_ntoa( *(IN_ADDR*)ptr->h_addr_list[0] ), strlen( inet_ntoa( *(IN_ADDR*)ptr->h_addr_list[0] ) ) );
		break;
	default:
		printf( "����ε� ��ȣ�� �Է����ּ���." );
		break;
	}

	printf( "��Ʈ��ȣ�� �����ּ��� ex)80-81\n->" );
	scanf( "%d-%d", &sNum, &eNum );
	printf( "\n" );


	getPortOnOff( domainName, sNum, eNum );

	WSACleanup();

	return 0;
}









/*
h_addr_list �ȿ� char������ �ּҸ� ��� �־, IN_ADDR ����ü�� �ٲ۵� inet_ntoad�� s_addr�� �����ϱ� ���ؼ�
*/