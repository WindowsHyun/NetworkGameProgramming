#include <windows.h>
#include <stdio.h>

#define MAXCNT 100000
/*
maxcnt를 한번에 처리할수 있는 범위까지 숫자를 낮추면 임계영역을 쓰나 안쓰나 동일할 수 있다.
그래서 일부로 maxcnt를 높여서 임계영역을 쓰는 이유를 볼 수 있다.
임계영역은 쓰레드 자체를 활당받는것을 멈출수가 없다. 그래서 임계영역을 안쓴것과 차이가 더 클 수 있다.
이럴 경우에는 이벤트를 사용하는것이 오히려 더 좋을 수 있다.
*/
int g_count = 0;
CRITICAL_SECTION cs;

DWORD WINAPI MyThread1( LPVOID arg )
{
	for ( int i = 0; i<MAXCNT; i++ ) {
		EnterCriticalSection( &cs );
		g_count += 2;
		//printf( "Thread1 : %d\n", g_count );
		LeaveCriticalSection( &cs );
	}
	return 0;
}

DWORD WINAPI MyThread2( LPVOID arg )
{
	for ( int i = 0; i<MAXCNT; i++ ) {
		EnterCriticalSection( &cs );
		g_count -= 2;
		//printf( "Thread2 : %d\n", g_count );
		LeaveCriticalSection( &cs );
	}
	return 0;
}

int main( int argc, char *argv[] )
{
	// 임계 영역 초기화
	InitializeCriticalSection( &cs );
	// 두 개의 스레드 생성
	HANDLE hThread[2];
	hThread[0] = CreateThread( NULL, 0, MyThread1, NULL, 0, NULL );
	hThread[1] = CreateThread( NULL, 0, MyThread2, NULL, 0, NULL );
	// 두 개의 스레드 종료 대기
	WaitForMultipleObjects( 2, hThread, TRUE, INFINITE );
	// 임계 영역 삭제
	DeleteCriticalSection( &cs );
	// 결과 출력
	printf( "g_count = %d\n", g_count );
	return 0;
}