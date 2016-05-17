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

//加密模块
//参数:曲线名、公钥x、公钥y、待加密信息、信息长度、密文长度
extern unsigned char* encrypt(char *curve, char *pub_x, char *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte);