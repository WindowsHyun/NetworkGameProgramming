#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <atlstr.h> /* cstring 사용을 위해서 사용하는 h 파일 */
#include <CommCtrl.h> /* PBM_SETPOS 프로그래스바 사용을 위해서 사용하는 h 파일 클라에서 사용할꺼니깐 지우지 말자 */
#include "resource.h"

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512
#define DataBufSize    1024

// 윈도우 프로시저
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
BOOL CALLBACK DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

// 편집 컨트롤 출력 함수
void DisplayText( char *fmt, ... );

// 오류 출력 함수
void err_quit( char *msg );
void err_display( char *msg );

SOCKET sock; // 소켓
char contentBuf[BUFSIZE + 1]; //sprintf 등을 사용할때 쓰자
char fileName[MAX_PATH]; // 파일의 이름
HANDLE hReadEvent, hWriteEvent; // 이벤트
HWND hProgress, hSerarchButton, hSendButton, hTextBox; // 편집 컨트롤

DWORD WINAPI ProcessClient( LPVOID arg );

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {
	// 이벤트 생성
	hReadEvent = CreateEvent( NULL, FALSE, TRUE, NULL );
	if ( hReadEvent == NULL ) return 1;
	hWriteEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	if ( hWriteEvent == NULL ) return 1;

	// 소켓 통신 스레드 생성
	CreateThread( NULL, 0, ProcessClient, NULL, 0, NULL );

	// 대화상자 생성
	DialogBox( hInstance, MAKEINTRESOURCE( IDD_DIALOG1 ), NULL, DlgProc );

	// 이벤트 제거
	CloseHandle( hReadEvent );
	CloseHandle( hWriteEvent );

	// closesocket()
	closesocket( sock );

	// 윈속 종료
	WSACleanup();
	return 0;
}

BOOL CALLBACK DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	static OPENFILENAME OFN;
	static char lpstrFile[MAX_PATH];
	static char szTitle[100];
	static char str[256];
	switch ( uMsg ) {
	case WM_INITDIALOG:
		hTextBox = GetDlgItem( hDlg, IDC_EDIT1 );
		hProgress = GetDlgItem( hDlg, IDC_PROGRESS1 );
		hSerarchButton = GetDlgItem( hDlg, IDC_BUTTON1 );
		hSendButton = GetDlgItem( hDlg, IDC_BUTTON2 );
		SendMessage( hProgress, PBM_SETPOS, 100, 0 );
		//SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)"추가한다." );
		//SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)"Server is Ready..!" );
		return TRUE;


	case WM_COMMAND:
		switch ( LOWORD( wParam ) ) {

		case IDC_BUTTON1:
			memset( &OFN, 0, sizeof( OPENFILENAME ) ); // 초기화
			OFN.lStructSize = sizeof( OPENFILENAME );
			OFN.hwndOwner = hDlg;
			OFN.lpstrFilter = "모든 파일 (*.*)\0*.*\0";
			OFN.lpstrFile = lpstrFile;
			OFN.nMaxFile = MAX_PATH;
			OFN.lpstrFileTitle = fileName;
			OFN.nMaxFileTitle = MAX_PATH;
			OFN.lpstrInitialDir = "."; // 초기 디렉토리
			GetOpenFileName( &OFN );
			SetDlgItemText( hDlg, IDC_EDIT1, lpstrFile );
			return TRUE;

		case IDCANCEL:
			EndDialog( hDlg, IDCANCEL );
			return TRUE;

		}
		return FALSE;
	}
	return FALSE;
}

DWORD WINAPI ProcessClient( LPVOID arg ) {
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != 0 )
		return 1;

	// socket()
	sock = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sock == INVALID_SOCKET ) err_quit( "socket()" );

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory( &serveraddr, sizeof( serveraddr ) );
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr( SERVERIP );
	serveraddr.sin_port = htons( SERVERPORT );
	retval = connect( sock, (SOCKADDR *)&serveraddr, sizeof( serveraddr ) );
	if ( retval == SOCKET_ERROR ) err_quit( "connect()" );


	return 0;
}

// 소켓 함수 오류 출력 후 종료
void err_quit( char *msg ) {
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