#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <atlstr.h> /* cstring ����� ���ؼ� ����ϴ� h ���� */
#include <CommCtrl.h> /* PBM_SETPOS ���α׷����� ����� ���ؼ� ����ϴ� h ���� Ŭ�󿡼� ����Ҳ��ϱ� ������ ���� */
#include "resource.h"

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512
#define DataBufSize    1024

// ������ ���ν���
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
BOOL CALLBACK DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

// ���� ��Ʈ�� ��� �Լ�
void DisplayText( char *fmt, ... );

// ���� ��� �Լ�
void err_quit( char *msg );
void err_display( char *msg );

SOCKET sock; // ����
char contentBuf[MAX_PATH]; //sprintf ���� ����Ҷ� ����
TCHAR file_locale[MAX_PATH];
char fileName[MAX_PATH]; // ������ �̸�
HANDLE hReadEvent, hWriteEvent; // �̺�Ʈ
HWND ShareHwnd;
HWND hProgress, hSerarchButton, hSendButton, hTextBox, hTextBox2; // ���� ��Ʈ��

DWORD WINAPI ProcessClient( LPVOID arg );
DWORD WINAPI SendFile( LPVOID arg );

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {
	// �̺�Ʈ ����
	hReadEvent = CreateEvent( NULL, FALSE, TRUE, NULL );
	if ( hReadEvent == NULL ) return 1;
	hWriteEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	if ( hWriteEvent == NULL ) return 1;

	// ���� ��� ������ ����
	CreateThread( NULL, 0, ProcessClient, NULL, 0, NULL );

	// ��ȭ���� ����
	DialogBox( hInstance, MAKEINTRESOURCE( IDD_DIALOG1 ), NULL, DlgProc );

	// �̺�Ʈ ����
	CloseHandle( hReadEvent );
	CloseHandle( hWriteEvent );

	// closesocket()
	closesocket( sock );

	// ���� ����
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
		//SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)"�߰��Ѵ�." );
		//SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)"Server is Ready..!" );
		return TRUE;


	case WM_COMMAND:
		switch ( LOWORD( wParam ) ) {

		case IDC_BUTTON1:
			memset( &OFN, 0, sizeof( OPENFILENAME ) ); // �ʱ�ȭ
			OFN.lStructSize = sizeof( OPENFILENAME );
			OFN.hwndOwner = hDlg;
			OFN.lpstrFilter = "��� ���� (*.*)\0*.*\0";
			OFN.lpstrFile = lpstrFile;
			OFN.nMaxFile = MAX_PATH;
			OFN.lpstrFileTitle = fileName;
			OFN.nMaxFileTitle = MAX_PATH;
			OFN.lpstrInitialDir = "."; // �ʱ� ���丮
			GetOpenFileName( &OFN );
			EnableWindow( hSendButton, TRUE ); // ������ ��ư Ȱ��ȭ
			SetDlgItemText( hDlg, IDC_EDIT1, lpstrFile );
			return TRUE;

		case IDC_BUTTON2:
			// ���� ��ư�� ������� SendFile ������ Ȱ��ȭ

			GetWindowText( hTextBox, file_locale, 1024 );
			if ( !strcmp( file_locale, "" ) ) {
				MessageBox( hTextBox, "������ ���� ������ �ּ���.", "", 0 );
				EnableWindow( hSendButton, FALSE ); // ������ ��ư ��Ȱ��ȭ
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

	// ���� �ʱ�ȭ
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
	// ������ ��ſ� ����� ����
	FILE *fp;
	int retval;
	int fileSize;
	int sendSize;
	float startTime, endTime;

	if ( (fp = fopen( file_locale, "rb" )) == NULL ) {
		fputs( "���� ���� ����!\nerror : ", stderr );
		MessageBox( hTextBox, "���� �б� ����!", "TCPClient_GUI", 0 );
		return 0;
	}
	sprintf( contentBuf, "���� ������ �غ��� �Դϴ�..!" );
	SetDlgItemText( ShareHwnd, IDC_EDIT2, contentBuf );

	EnableWindow( hSendButton, FALSE ); // ������ ��ư ��Ȱ��ȭ

	fseek( fp, 0L, SEEK_END ); //���� ������ ��ġ �ű�
	fileSize = ftell( fp ); //���� ����Ʈ�� ���
	fseek( fp, 0L, SEEK_SET ); //�ٽ� ���� ó������ ��ġ �ű�
	//printf( "-----------------------------------------------\n" );
	//printf( "������ ũ�� : %d bytes \n", fileSize );
	//printf( "������ �̸� : %s\n", fileName );
	//printf( "-----------------------------------------------\n" );
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	int len = strlen( file_locale );
	char *namebuf = new char[len];
	strncpy( namebuf, file_locale, len );
	// ������ ������(���� ����)
	retval = send( sock, (char *)&len, sizeof( int ), 0 );
	if ( retval == SOCKET_ERROR ) {
		err_display( "send()" );
		exit( 1 );
	}
	// ������ ������(���� ����)
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

	sprintf( contentBuf, "���� ������ �����Ͽ����ϴ�..!" );
	SetDlgItemText( ShareHwnd, IDC_EDIT2, contentBuf );

	startTime = GetTickCount();
	while ( sendData ) {
		//���� �о ���ۿ� ����
		if ( sendSize <= DataBufSize ) {
			bufSize = DataBufSize - sendSize;
			sendData = false;
		}
		else {
			bufSize = DataBufSize;
		}

		fread( buf, 1, bufSize, fp );

		//����
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
	//printf( "�� �ҿ�ð� : %f��\n", (endTime - startTime) / 1024 );

	sprintf( contentBuf, "�� �ҿ�ð� : %f��", (endTime - startTime) / 1024 );
	SetDlgItemText( ShareHwnd, IDC_EDIT2, contentBuf );
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	EnableWindow( hSendButton, TRUE ); // ������ ��ư ��Ȱ��ȭ
	MessageBox( hTextBox, "������ �Ϸ��Ͽ����ϴ�..!", "TCPClient_GUI", 0 );

	return 0;
}

// ���� �Լ� ���� ��� �� ����
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