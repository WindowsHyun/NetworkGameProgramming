#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <stdio.h>
/*
시스템에서 사용하는 바이트 정렬 방식을 확인할 수 있도록 다음과 같은 함수를 정의하시오
*/

BOOL IsLittleEndian() {
	// 리턴 값 : 리틀 엔디안이면 True, 그렇지 않으면 FALSE를 리턴한다.
	// 리틀 엔디안은 뒤에서 부터 읽고 쓴다
	int num = 0x41470566; 
	if ( *((char*)&num) == 0x66 ) { 
		return true;
	}
	else {
		return false;
	}
}

BOOL IsBigEndian() {
	// 리턴 값 : 빅 엔디안 이면 True, 그렇지 않으면 False를 리턴한다.
	// 빅엔디안은 앞에서 부터 읽고 쓴다.
	int num = 0x41470566;
	if ( *((char*)&num) == 0x41 ) {
		return true;
	}
	else {
		return false;
	}
}

int main( ) {
	
	if ( IsLittleEndian() ) {
		printf( "LittleEndian 입니다..!\n" );
	}
	else if ( IsBigEndian() ) {
		printf( "BigEndian 입니다..!\n" );
	}


	return 0;
}

/*
0x41470566 16진수를 int 4바이트에 넣어서
char로 1바이트로 불러올시 66 05 47 41 이렇게 끊어져서 나온다.
num의 주소를 char 포인터 형으로 변환하면 *연산자로 역참조를 하였을 때 num의 전체 바이트 중에 1바이트를 참조하고 있을것입니다, 
*/