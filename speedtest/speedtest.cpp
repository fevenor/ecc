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

unsigned char* encrypttest(char const *curve, char const *pub_x, char const *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte)
{
	unsigned char* cipher = (unsigned char*)malloc(sizeof(unsigned char));
	for (int i = 1; i <= 4; i++)
	{
		free(cipher);
		cout << i << "st time encrypt:\t";
		auto begin_time = chrono::high_resolution_clock::now();
		cipher = ecc_encrypt(curve, pub_x, pub_y, info, info_length_byte, cipherdata_length_byte);
		auto end_time = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - begin_time).count();
		cout << duration << "ms\t";
		cout << setiosflags(ios::fixed) << setprecision(3) << (float)(info_length_byte / duration) * 1000 / 1048576 << "MB/s" << endl;
	}
	return cipher;
}


void decrypttest(char const *key, unsigned char *secret, unsigned long long cipherdata_length_byte)
{
	int flag = -1;
	unsigned long long plaindata_length_byte = 0;
	for (int i = 1; i <= 4; i++)
	{
		cout << i << "st time decrypt:\t";
		auto begin_time = chrono::high_resolution_clock::now();
		ecc_decrypt(key, secret, cipherdata_length_byte, &plaindata_length_byte, &flag);
		auto end_time = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - begin_time).count();
		cout << duration << "ms\t";
		cout << setiosflags(ios::fixed) << setprecision(3) << (float)(plaindata_length_byte / duration) * 1000 / 1048576 << "MB/s" << endl;
	}
}


int main()
{
	int i;
	//生成测试密钥对
	string curvename[4] = { "secp160r2" ,"secp192r1" ,"secp224r1" ,"secp256r1" };
	char privatekey[4][65], public_x[4][65], public_y[4][65];
	for (i = 0; i < 4; i++)
	{
		get_key(curvename[i].c_str(), privatekey[i], public_x[i], public_y[i]);
		cout << "curvename:" << curvename[i].c_str() << endl;
		cout << "privatekey:" << endl << privatekey[i] << endl;
		cout << "publickey:" << endl << public_x[i] << endl << public_y[i] << endl << endl;
	}

	//生成随机数据
	size_t size1 = 1024 * 1024 * 1;
	size_t size10 = 1024 * 1024 * 10;
	size_t size100 = 1024 * 1024 * 100;
	BYTE* data1 = getrandomdata(size1);
	BYTE* data10 = getrandomdata(size10);
	BYTE* data100 = getrandomdata(size100);


	//测试曲线长度的影响
	cout << "测试曲线长度的影响:" << endl;
	unsigned long long cipherdata_length_byte = 0;
	for (i = 0; i < 4; i++)
	{
		cout << curvename[i].c_str() << endl;
		unsigned char* cipher = encrypttest(curvename[i].c_str(), public_x[i], public_y[i], data100, (unsigned long long)size100, &cipherdata_length_byte);
		decrypttest(privatekey[i], cipher, cipherdata_length_byte);
		delete[] cipher;
		cout << endl;
	}



	//测试分块大小的影响
	cout << "测试分块大小的影响(使用" << curvename[3].c_str() << "曲线):" << endl;
	cout << "1MByte:" << endl;
	unsigned long long cipherdata_length_byte1 = 0;
	unsigned char* cipher1 = encrypttest(curvename[3].c_str(), public_x[3], public_y[3], data1, (unsigned long long)size1, &cipherdata_length_byte1);
	unsigned long long cipherdata_length_byte10 = 0;
	cout << "10MBytes:" << endl;
	unsigned char* cipher10 = encrypttest(curvename[3].c_str(), public_x[3], public_y[3], data10, (unsigned long long)size10, &cipherdata_length_byte10);
	cout << "100MBytes:" << endl;
	unsigned long long cipherdata_length_byte100 = 0;
	unsigned char* cipher100 = encrypttest(curvename[3].c_str(), public_x[3], public_y[3], data100, (unsigned long long)size100, &cipherdata_length_byte100);
	cout << endl;

	cout << "1MByte:" << endl;
	decrypttest(privatekey[3], cipher1, cipherdata_length_byte1);
	cout << "10MBytes:" << endl;
	decrypttest(privatekey[3], cipher10, cipherdata_length_byte10);
	cout << "100MBytes:" << endl;
	decrypttest(privatekey[3], cipher100, cipherdata_length_byte100);

	delete[] data1;
	delete[] data10;
	delete[] data100;
	delete[] cipher1;
	delete[] cipher10;
	delete[] cipher100;
	system("pause");
	return 0;
}