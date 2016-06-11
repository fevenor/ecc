#pragma once
#include <fstream>
#include <string>
#include <vector>
#include "../basefunc/basefunc.h"
#pragma comment(lib,"..\\..\\ecc\\x64\\Release\\basefunc.lib")

//������������Կ
extern _declspec(dllexport) int gkey(char const *curve, char *privatekey, char *public_x, char *public_y);
//�ı�����
extern _declspec(dllexport) unsigned char* textencrypt(char const *curve, char const *pub_x, char const *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte);
//�ļ�����
extern _declspec(dllexport) int fileencrypt(char const *curve, char const *pub_x, char const *pub_y, char const *in, char const *out);
//�ı�����
extern _declspec(dllexport) unsigned char* textdecrypt(char const *key, unsigned char *secret, unsigned long long cipherdata_length_byte, unsigned long long *plaindata_length_byte, int *flag);
//�ļ�����
extern _declspec(dllexport) int filedecrypt(char const *key, char const *in, char const *out);
