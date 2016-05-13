#pragma once
#include "..\\libcal\libcal.h"
#if defined _MSC_VER								//�ж�visual studio����
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