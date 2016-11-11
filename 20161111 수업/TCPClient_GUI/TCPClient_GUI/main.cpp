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
char contentBuf[MAX_PATH]; //sprintf 등을 사용할때 쓰자
TCHAR file_locale[MAX_PATH];
char fileName[MAX_PATH]; // 파일의 이름
HANDLE hReadEvent, hWriteEvent; // 이벤트
HWND ShareHwnd;
HWND hProgress, hSerarchButton, hSendButton, hTextBox, hTextBox2; // 편집 컨트롤

DWORD WINAPI ProcessClient( LPVOID arg );
DWORD WINAPI SendFile( LPVOID arg );

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
	ShareHwnd = hDlg;
	static OPENFILENAME OFN;
	static char lpstrFile[MAX_PATH];
	static char szTitle[100];
	static char str[256];
	switch ( uMsg ) {
	case WM_INITDIALOG:
		hTextBox = GetDlgItem( hDlg, IDC_EDIT1 );
		hTextBox2 = GetDlgItem( hDlg, IDC_EDIT2 );
		hProgress = GetDlgItem( hDlg, IDC_PROGRESS1 );
		hSerarchButton = GetDlgItem( hDlg, IDC_BUTTON1 );
		hSendButton = GetDlgItem( hDlg, IDC_BUTTON2 );
		SendMessage( hProgress, PBM_SETPOS, 0, 0 );
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
			EnableWindow( hSendButton, TRUE ); // 보내기 버튼 활성화
			SetDlgItemText( hDlg, IDC_EDIT1, lpstrFile );
			return TRUE;

		case IDC_BUTTON2:
			// 전송 버튼을 누를경우 SendFile 스레드 활성화

			GetWindowText( hTextBox, file_locale, 1024 );
			if ( !strcmp( file_locale, "" ) ) {
				MessageBox( hTextBox, "파일을 먼저 선택해 주세요.", "", 0 );
				EnableWindow( hSendButton, FALSE ); // 보내기 버튼 비활성화
			}
			else {
				CreateThread( NULL, 0, SendFile, NULL, 0, NULL );
			}
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

DWORD WINAPI SendFile( LPVOID arg ) {
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// 데이터 통신에 사용할 변수
	FILE *fp;
	int retval;
	int fileSize;
	int sendSize;
	float startTime, endTime;

	if ( (fp = fopen( file_locale, "rb" )) == NULL ) {
		fputs( "파일 열기 에러!\nerror : ", stderr );
		MessageBox( hTextBox, "파일 읽기 에러!", "TCPClient_GUI", 0 );
		return 0;
	}
	sprintf( contentBuf, "파일 전송을 준비중 입니다..!" );
	SetDlgItemText( ShareHwnd, IDC_EDIT2, contentBuf );

	EnableWindow( hSendButton, FALSE ); // 보내기 버튼 비활성화

	fseek( fp, 0L, SEEK_END ); //파일 끝으로 위치 옮김
	fileSize = ftell( fp ); //파일 바이트값 출력
	fseek( fp, 0L, SEEK_SET ); //다시 파일 처음으로 위치 옮김
	//printf( "-----------------------------------------------\n" );
	//printf( "데이터 크기 : %d bytes \n", fileSize );
	//printf( "데이터 이름 : %s\n", fileName );
	//printf( "-----------------------------------------------\n" );
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	int len = strlen( file_locale );
	char *namebuf = new char[len];
	strncpy( namebuf, file_locale, len );
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
	bool sendData = true;
	sendSize = fileSize;

	retval = send( sock, (char *)&fileSize, sizeof( int ), 0 );
	if ( retval == SOCKET_ERROR ) {
		err_display( "send()" );
		exit( 1 );
	}

	sprintf( contentBuf, "파일 전송을 시작하였습니다..!" );
	SetDlgItemText( ShareHwnd, IDC_EDIT2, contentBuf );

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
		SendMessage( hProgress, PBM_SETPOS, nowPer, 0 );
	}
	endTime = GetTickCount();
	//printf( " ]\n" );
	//printf( "총 소요시간 : %f초\n", (endTime - startTime) / 1024 );

	sprintf( contentBuf, "총 소요시간 : %f초", (endTime - startTime) / 1024 );
	SetDlgItemText( ShareHwnd, IDC_EDIT2, contentBuf );
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	EnableWindow( hSendButton, TRUE ); // 보내기 버튼 비활성화
	MessageBox( hTextBox, "전송을 완료하였습니다..!", "TCPClient_GUI", 0 );

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

void err_display( char *msg ){
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		(LPTSTR)&lpMsgBuf, 0, NULL );
	sprintf( contentBuf, "[%s] %s", msg, (char *)lpMsgBuf );
	SetDlgItemText( ShareHwnd, IDC_EDIT2, contentBuf );
	LocalFree( lpMsgBuf );
}