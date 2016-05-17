#pragma once
#include "..\\libcal\libcal.h"
#if defined _MSC_VER								//�ж�visual studio����
#include "windows.h"
#include "process.h"
//#pragma comment(lib,"..\\x64\\Release\\libcal.lib")	//���������
#endif

//��Կ��Ϣ
typedef struct
{
	char *curve;
	char *private;
	char *public_x;
	char *public_y;
}key;


//���ָ�����ȵ������k
extern void get_rand(int length, mpz_t k);
//������������Կ
extern key* get_key(enum curve_name ecname);

//����ģ��
//����:����������Կx����Կy����������Ϣ����Ϣ���ȡ����ĳ���
extern unsigned char* encrypt(char *curve, char *pub_x, char *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte);

//����ģ��
//����:˽Կ����������Ϣ�����ĳ��ȡ����ĳ���
extern unsigned char* decrypt(char *key, unsigned char *secret, unsigned long long cipherdata_length_byte, unsigned long long *plaindata_length_byte);
