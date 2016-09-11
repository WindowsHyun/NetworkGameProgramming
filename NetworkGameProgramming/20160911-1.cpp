#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <stdio.h>
/*
�ý��ۿ��� ����ϴ� ����Ʈ ���� ����� Ȯ���� �� �ֵ��� ������ ���� �Լ��� �����Ͻÿ�
*/

BOOL IsLittleEndian() {
	// ���� �� : ��Ʋ ������̸� True, �׷��� ������ FALSE�� �����Ѵ�.
	// ��Ʋ ������� �ڿ��� ���� �а� ����
	int num = 0x41470566; 
	if ( *((char*)&num) == 0x66 ) { 
		return true;
	}
	else {
		return false;
	}
}

BOOL IsBigEndian() {
	// ���� �� : �� ����� �̸� True, �׷��� ������ False�� �����Ѵ�.
	// �򿣵���� �տ��� ���� �а� ����.
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
		printf( "LittleEndian �Դϴ�..!\n" );
	}
	else if ( IsBigEndian() ) {
		printf( "BigEndian �Դϴ�..!\n" );
	}


	return 0;
}

/*
0x41470566 16������ int 4����Ʈ�� �־
char�� 1����Ʈ�� �ҷ��ý� 66 05 47 41 �̷��� �������� ���´�.
num�� �ּҸ� char ������ ������ ��ȯ�ϸ� *�����ڷ� �������� �Ͽ��� �� num�� ��ü ����Ʈ �߿� 1����Ʈ�� �����ϰ� �������Դϴ�, 
*/