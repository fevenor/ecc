#pragma once
#include "..\\libcal\libcal.h"
#if defined _MSC_VER								//判断visual studio环境
//#pragma comment(lib,"..\\x64\\Release\\libcal.lib")	//基础运算库
#endif

//密钥信息
typedef struct
{
	char *curve;
	char *private;
	char *public_x;
	char *public_y;
}key;


//获得指定长度的随机数k
extern void get_rand(int length, mpz_t k);
//由曲线生成密钥
extern key* get_key(enum curve_name ecname);