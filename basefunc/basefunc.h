#pragma once
#include "..\\libcal\libcal.h"
#if defined _MSC_VER								//判断visual studio环境
#include "windows.h"
#include "process.h"
#pragma comment(lib, "..\\mpir\\mpir.lib")	//MPIR静态链接库
#pragma comment(lib,"..\\x64\\Release\\libcal.lib")	//基础运算库
#endif


//获得指定长度的随机数k
extern void get_rand(int length, mpz_t k);
//由曲线生成密钥
extern _declspec(dllexport) int get_key(char *curve, char *privatekey, char *public_x, char *public_y);

//加密模块
//参数:曲线名、公钥x、公钥y、待加密信息、信息长度、密文长度
extern _declspec(dllexport) unsigned char* encrypt(char *curve, char *pub_x, char *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte);

//解密模块
//参数:私钥、待解密信息、密文长度、明文长度
extern _declspec(dllexport) unsigned char* decrypt(char *key, unsigned char *secret, unsigned long long cipherdata_length_byte, unsigned long long *plaindata_length_byte);
