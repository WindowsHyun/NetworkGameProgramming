#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <atlstr.h> /* cstring ����� ���ؼ� ����ϴ� h ���� */
//#include <CommCtrl.h> /* PBM_SETPOS ���α׷����� ����� ���ؼ� ����ϴ� h ���� Ŭ�󿡼� ����Ҳ��ϱ� ������ ���� */
#include "resource.h"

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
char contentBuf[BUFSIZE + 1]; //sprintf ���� ����Ҷ� ����
HANDLE hReadEvent, hWriteEvent; // �̺�Ʈ
HWND hSendButton; // ������ ��ư
HWND hList, hButton; // ���� ��Ʈ��

DWORD WINAPI ProcessClient( LPVOID arg ); // Ŭ�� ������ ���� ������
DWORD WINAPI Thread_Server( LPVOID arg ); // ������ ���� ������ �ޱ����� ������
int recvn( SOCKET s, char *buf, int len, int flags );

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
	switch ( uMsg ) {
	case WM_INITDIALOG:
		//hProgress = GetDlgItem( hDlg, IDC_PROGRESS1 );
		hList = GetDlgItem( hDlg, IDC_LIST1 );
		hButton = GetDlgItem( hDlg, IDC_BUTTON1 );
		//SendMessage( hProgress, PBM_SETPOS, 0, 0 );
		//SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)"�߰��Ѵ�." );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)"Server is Ready..!" );
		return TRUE;


	case WM_COMMAND:


		switch ( LOWORD( wParam ) ) {
		case IDC_LIST1:
			switch ( HIWORD( wParam ) ) {
			case LBN_SELCHANGE:
				int listIndex = SendMessage( hList, LB_GETCURSEL, 0, 0 ); //������ ����Ʈ ��ȣ�� ���Ѵ�.
				SendMessage( hList, LB_GETTEXT, listIndex, (LPARAM)contentBuf ); // ���� ��ȣ�� ����Ʈ�� ������ str�� �����Ѵ�.
				MessageBox( hList, contentBuf, "TCPSever_GUI", 0 );
				break;
			}
			return TRUE;

		case IDC_BUTTON1:
			SendMessage( hList, LB_RESETCONTENT, 0, 0 ); //����Ʈ ������ �ʱ�ȭ ���ش�.
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

	// ������ ��ſ� ����� ����
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

		// ������ Ŭ���̾�Ʈ ���� ���
		sprintf( contentBuf, "[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d", inet_ntoa( clientaddr.sin_addr ), ntohs( clientaddr.sin_port ) );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );

		// ������ ����
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

	// ���� ����
	WSACleanup();
	return 0;
}

DWORD WINAPI Thread_Server( LPVOID arg ) {
	SOCKET client_sock = (SOCKET)arg;
	SOCKADDR_IN clientaddr;
	int addrlen;
	int retval;
	int len;
	int reciveSize; //���� ������� ������ ���ؼ� ���� ��ŭ ���������� Ȯ���� ���ؼ�
	int nowreciveSize;
	int percentage;
	int fileSize;
	float nowPer;
	float startTime, endTime;
	char filename[BUFSIZE];

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof( clientaddr );
	getpeername( client_sock, (SOCKADDR *)&clientaddr, &addrlen );

	while ( 1 ) {
		//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// ���ϸ� �޾ƿ���
		retval = recvn( client_sock, (char *)&len, sizeof( int ), 0 ); // ������ �ޱ�(���� ����)
		if ( retval == SOCKET_ERROR ) {
			err_display( "recv()" );
			break;
		}
		else if ( retval == 0 )
			break;

		char *buf = new char[len]; // ���۵� ���̸� �˰� ������ ũ�⿡ ���缭 buf�� �÷�����!

		retval = recvn( client_sock, buf, len, 0 ); // ������ �ޱ�(���� ����)
		if ( retval == SOCKET_ERROR ) {
			err_display( "recv()" );
			break;
		}
		else if ( retval == 0 )
			break;

		// ���� ������ ���
		buf[retval] = '\0';
		sprintf( filename, "%s", buf );

		CString str = filename;
		CString fileexe = str.Right( str.GetLength() - str.ReverseFind( '\\' ) - 1 );


		//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// ���� ���� �����͸� �ޱ�
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
		sprintf( contentBuf, "���� �� : %s [%d byte]", fileexe, reciveSize );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		sprintf( contentBuf, "�ۼ������� : 0%%" );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		sprintf( contentBuf, "-----------------------------------------------" );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );



		char buffer[DataBufSize]; // ���۵� ���̸� �˰� ������ ũ�⿡ ���缭 buffer�� �÷�����!
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
				sprintf( contentBuf, "���� �� : %s [%d byte]", fileexe, reciveSize );
				SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
				sprintf( contentBuf, "�ۼ������� : %f%%", nowPer );
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
		sprintf( contentBuf, "���� �� : %s [%d byte]", fileexe, reciveSize );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		sprintf( contentBuf, "�ۼ������� : 100%%" );
		SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)contentBuf );
		sprintf( contentBuf, "�� �ҿ�ð� : %f��", (endTime - startTime) / 1024 );
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

// ���� �Լ� ���� ���
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

// ����� ���� ������ ���� �Լ�
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