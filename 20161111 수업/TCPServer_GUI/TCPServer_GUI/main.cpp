#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <atlstr.h> /* cstring 사용을 위해서 사용하는 h 파일 */
//#include <CommCtrl.h> /* PBM_SETPOS 프로그래스바 사용을 위해서 사용하는 h 파일 클라에서 사용할꺼니깐 지우지 말자 */
#include "resource.h"

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
HANDLE hReadEvent, hWriteEvent; // 이벤트
HWND hSendButton; // 보내기 버튼
HWND hList, hButton; // 편집 컨트롤

DWORD WINAPI ProcessClient( LPVOID arg ); // 클라 접속을 위한 스레드
DWORD WINAPI Thread_Server( LPVOID arg ); // 접속후 파일 전송을 받기위한 스레드
int recvn( SOCKET s, char *buf, int len, int flags );

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
	switch ( uMsg ) {
	case WM_INITDIALOG:
		//hProgress = GetDlgItem( hDlg, IDC_PROGRESS1 );
		hList = GetDlgItem( hDlg, IDC_LIST1 );
		hButton = GetDlgItem( hDlg, IDC_BUTTON1 );
		//SendMessage( hProgress, PBM_SETPOS, 0, 0 );
		//SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)"추가한다." );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)"Server is Ready..!" );
		return TRUE;


	case WM_COMMAND:


		switch ( LOWORD( wParam ) ) {
		case IDC_LIST1:
			switch ( HIWORD( wParam ) ) {
			case LBN_SELCHANGE:
				int listIndex = SendMessage( hList, LB_GETCURSEL, 0, 0 ); //현재의 리스트 번호를 구한다.
				SendMessage( hList, LB_GETTEXT, listIndex, (LPARAM)contentBuf ); // 구한 번호의 리스트의 내용을 str에 복사한다.
				MessageBox( hList, contentBuf, "TCPSever_GUI", 0 );
				break;
			}
			return TRUE;

		case IDC_BUTTON1:
			SendMessage( hList, LB_RESETCONTENT, 0, 0 ); //리스트 내용을 초기화 해준다.
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
		sprintf( contentBuf, "[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d", inet_ntoa( clientaddr.sin_addr ), ntohs( clientaddr.sin_port ) );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );

		// 스레드 생성
		hThread = CreateThread( NULL, 0, Thread_Server, (LPVOID)client_sock, 0, NULL );
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

DWORD WINAPI Thread_Server( LPVOID arg ) {
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

		retval = recvn( client_sock, buf, len, 0 ); // 데이터 받기(가변 길이)
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

		sprintf( contentBuf, "-----------------------------------------------" );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		sprintf( contentBuf, "Client IP : %s, Port : %d", inet_ntoa( clientaddr.sin_addr ), ntohs( clientaddr.sin_port ) );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		sprintf( contentBuf, "파일 명 : %s [%d byte]", fileexe, reciveSize );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		sprintf( contentBuf, "퍼센테이지 : 0%%" );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		sprintf( contentBuf, "-----------------------------------------------" );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );



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
				sprintf( contentBuf, "-----------------------------------------------" );
				SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
				sprintf( contentBuf, "Client IP : %s, Port : %d", inet_ntoa( clientaddr.sin_addr ), ntohs( clientaddr.sin_port ) );
				SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
				sprintf( contentBuf, "파일 명 : %s [%d byte]", fileexe, reciveSize );
				SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
				sprintf( contentBuf, "퍼센테이지 : %f%%", nowPer );
				SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
				sprintf( contentBuf, "-----------------------------------------------" );
				SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
			}

		}
		endTime = GetTickCount();

		sprintf( contentBuf, "-----------------------------------------------" );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		sprintf( contentBuf, "Client IP : %s, Port : %d", inet_ntoa( clientaddr.sin_addr ), ntohs( clientaddr.sin_port ) );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		sprintf( contentBuf, "파일 명 : %s [%d byte]", fileexe, reciveSize );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		sprintf( contentBuf, "퍼센테이지 : 100%%" );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		sprintf( contentBuf, "총 소요시간 : %f초", (endTime - startTime) / 1024 );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		sprintf( contentBuf, "Download the complete file!", (endTime - startTime) / 1024 );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		sprintf( contentBuf, "-----------------------------------------------" );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		fclose( fp );

	}
	closesocket( client_sock );

	return 0;
}

// 소켓 함수 오류 출력
void err_display( char *msg ) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		(LPTSTR)&lpMsgBuf, 0, NULL );
	DisplayText( "[%s] %s", msg, (char *)lpMsgBuf );
	LocalFree( lpMsgBuf );
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

void DisplayText( char *fmt, ... ) {
	va_list arg;
	va_start( arg, fmt );

	char cbuf[BUFSIZE + 256];
	vsprintf( cbuf, fmt, arg );

	SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)cbuf );

	/*int nLength = GetWindowTextLength( hEdit2 );
	SendMessage( hEdit2, EM_SETSEL, nLength, nLength );
	SendMessage( hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf );
	*/
	va_end( arg );
}

// 사용자 정의 데이터 수신 함수
int recvn( SOCKET s, char *buf, int len, int flags ) {
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