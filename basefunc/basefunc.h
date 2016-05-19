#pragma once
#include "..\\libcal\libcal.h"
#if defined _MSC_VER								//�ж�visual studio����
#include "windows.h"
#include "process.h"
#pragma comment(lib, "..\\mpir\\mpir.lib")	//MPIR��̬���ӿ�
#pragma comment(lib,"..\\x64\\Release\\libcal.lib")	//���������
#endif


//���ָ�����ȵ������k
extern void get_rand(int length, mpz_t k);
//������������Կ
extern _declspec(dllexport) int get_key(char *curve, char *privatekey, char *public_x, char *public_y);

//����ģ��
//����:����������Կx����Կy����������Ϣ����Ϣ���ȡ����ĳ���
extern _declspec(dllexport) unsigned char* encrypt(char *curve, char *pub_x, char *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte);

//����ģ��
//����:˽Կ����������Ϣ�����ĳ��ȡ����ĳ���
extern _declspec(dllexport) unsigned char* decrypt(char *key, unsigned char *secret, unsigned long long cipherdata_length_byte, unsigned long long *plaindata_length_byte);
