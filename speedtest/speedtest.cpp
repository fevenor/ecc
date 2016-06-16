#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include "../basefunc/basefunc.h"
#include <windows.h>
#if defined _DEBUG
#pragma comment(lib,"..\\x64\\Debug\\basefunc.lib")
#else
#pragma comment(lib,"..\\x64\\Release\\basefunc.lib")
#endif

#define sessionsnum 1000

using namespace std;

BYTE* getrandomdata(size_t length)
{
	HCRYPTPROV hProvider = 0;
	BYTE *buf = new BYTE[length];
	CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
	CryptGenRandom(hProvider, length, buf);
	CryptReleaseContext(hProvider, 0);
	return buf;
}

unsigned char* encrypttest(char *curve, char *pub_x, char *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte)
{
	unsigned char* cipher = (unsigned char*)malloc(sizeof(unsigned char));
	for (int i = 1; i <= 4; i++)
	{
		free(cipher);
		cout << i << "st time encrypt:\t";
		auto begin_time = chrono::high_resolution_clock::now();
		cipher = encrypt(curve, pub_x, pub_y, info, info_length_byte, cipherdata_length_byte);
		auto end_time = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - begin_time).count();
		cout << duration << "ms\t";
		cout << setiosflags(ios::fixed) << setprecision(3) << (float)(info_length_byte / duration) * 1000 / 1024 << "KB/s" << endl;
	}
	return cipher;
}



int main()
{
	int i;
	//生成测试密钥对
	string curvename[4] = { "secp160r2" ,"secp192r1" ,"secp224r1" ,"secp256r1" };
	char privatekey[4][65], public_x[4][65], public_y[4][65];
	for (i = 0; i < 4; i++)
	{
		get_key((char*)curvename[i].c_str(), privatekey[i], public_x[i], public_y[i]);
		cout << "curvename:" << curvename[i].c_str() << endl;
		cout << "privatekey:" << endl << privatekey[i] << endl;
		cout << "publickey:" << endl << public_x[i] << endl << public_y[i] << endl << endl;
	}

	//随机数据长度
	size_t size[4] = { 2 * (160 / 8 - 1) * sessionsnum - 8,2 * (192 / 8 - 1) * sessionsnum - 8,2 * (224 / 8 - 1) * sessionsnum - 8,2 * (256 / 8 - 1) * sessionsnum - 8 };


	//测试曲线长度的影响
	cout << "测试曲线长度的影响:" << endl;
	unsigned long long cipherdata_length_byte = 0;
	for (i = 0; i < 4; i++)
	{
		cout << curvename[i].c_str() << endl;
		BYTE* data = getrandomdata(size[i]);
		encrypttest((char*)curvename[i].c_str(), public_x[i], public_y[i], data, (unsigned long long)size[i], &cipherdata_length_byte);
		delete[] data;
		cout << endl;
	}
	system("pause");
	return 0;
}