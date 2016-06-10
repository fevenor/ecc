#pragma once
#include "../libcal/libcal.h"
#include <string.h>
#define sha1_len 20
#if defined _MSC_VER							//在windows下
#include <windows.h>
#include <process.h>
#pragma comment(lib, "..\\mpir\\mpir.lib")	//MPIR静态链接库
#pragma comment(lib,"..\\x64\\Release\\libcal.lib")	//基础运算库
//计算SHA1
void SHA1(unsigned char *info, unsigned long long info_length_byte, unsigned char *hash);
#elif defined __GNUC__					//在linux下
#include <unistd.h>
#include <pthread.h>
#include <openssl/sha.h>
#endif

//获得指定长度的随机数k
extern void get_rand(int length, mpz_t k);

#ifdef __cplusplus
extern "C" { int get_key(char const *curve, char *privatekey, char *public_x, char *public_y); }
extern "C" { unsigned char* ecc_encrypt(char const *curve, char const *pub_x, char const *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte); }
extern "C" { unsigned char* ecc_decrypt(char const *key, unsigned char *secret, unsigned long long cipherdata_length_byte, unsigned long long *plaindata_length_byte, int *flag); }
#else
//由曲线生成密钥
extern int get_key(char const *curve, char *privatekey, char *public_x, char *public_y);

//加密模块
//参数:曲线名、公钥x、公钥y、待加密信息、信息长度、密文长度
extern unsigned char* ecc_encrypt(char const *curve, char const *pub_x, char const *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte);

//解密模块
//参数:私钥、待解密信息、密文长度、明文长度
extern unsigned char* ecc_decrypt(char const *key, unsigned char *secret, unsigned long long cipherdata_length_byte, unsigned long long *plaindata_length_byte, int *flag);
#endif
