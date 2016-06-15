#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include "../basefunc/basefunc.h"

#if defined _MSC_VER								//判断visual studio环境
#include "getopt.h"
#include <windows.h>
#pragma comment(lib,"..\\..\\ecc\\x64\\Release\\basefunc.lib")
#elif defined __GNUC__
#include <getopt.h>
#endif

using namespace std;
//#if defined _MSC_VER
//extern "C" _declspec(dllimport) int get_key(const char *curve, char *privatekey, char *public_x, char *public_y);
//extern "C" _declspec(dllimport) unsigned char* ecc_encrypt(const char *curve, const char *pub_x, const char *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte);
//extern "C" _declspec(dllimport) unsigned char* ecc_decrypt(const char *key, unsigned char *secret, unsigned long long cipherdata_length_byte, unsigned long long *plaindata_length_byte, int *flag);
//#endif

// string转wstring
std::wstring StringToWstring(const std::string str)
{
	unsigned len = str.size() * 2;// 预留字节数
	setlocale(LC_CTYPE, "");     //必须调用此函数
	wchar_t *p = new wchar_t[len];// 申请一段内存存放转换后的字符串
	mbstowcs(p, str.c_str(), len);// 转换
	std::wstring str1(p);
	delete[] p;// 释放申请的内存
	return str1;
}

void display_help()
{
	cout << endl
		<< "用法：\n"
		<< "密钥对生成：\n"
		<< " ecc -g|--getkey [-n|--name curve_name] -k|--privatekey privatekey_file [-p|--publickey publickey_file]\n"
		<< "加密：\n"
		<< " ecc -e|--encrypt -p|--publickey publickey_file -i|--in file [-o|--out encrypted_file]\n"
		<< "解密：\n"
		<< " ecc -d|--decrypt -k|--privatekey privatekey_file -i|--in encrypted_file [-o|--out decrypted_file]\n"
		<< endl
		<< "选项：\n"
		<< "-g, --getkey\t\t密钥对生成\n"
		<< "-n, --name\t\t曲线名\n"
		<< "-e, --encrypt\t\t加密\n"
		<< "-d, --decrypt\t\t加密\n"
		<< "-k, --privatekey\t私钥文件\n"
		<< "-p, --publickey\t\t公钥文件\n"
		<< "-i, --in\t\t输入文件\n"
		<< "-o, --out\t\t输出文件\n"
		<< endl
		<< "支持的曲线：\n"
		<< "secp160r2\tsecp192r1\tsecp224r1\tsecp256r1\n"
		<< endl;
}

int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		display_help();
		return 1;
	}
	opterr = 0;
	char const *short_options = "gn:edp:k:i:o:";
	struct option long_options[] = {
		{ "getkey", 0, NULL, 'g' },
		{ "encrypt", 0, NULL, 'e' },
		{ "decrypt", 0, NULL, 'd' },
		{ "name", 1, NULL, 'n' },
		{ "privatekey", 1, NULL, 'k' },
		{ "publickey", 1, NULL, 'p' },
		{ "in", 1, NULL, 'i' },
		{ "out", 1, NULL, 'o' },
		{ 0, 0, 0, 0 }
	};
	int opt;
	int option_index = 0;
	bool getkey_sign = false, encrypt_sign = false, decrypt_sign = false;
	string curvename, privatekeyfilepath, publickeyfilepath, in, out;
	char privatekey[65], public_x[65], public_y[65];

	while ((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)	//读取选项
	{
		switch (opt)
		{
		case 'g':
			getkey_sign = true;
			break;
		case 'e':
			encrypt_sign = true;
			break;
		case 'd':
			decrypt_sign = true;
			break;
		case 'n':
			curvename = optarg;
			break;
		case 'k':
			privatekeyfilepath = optarg;
			break;
		case 'p':
			publickeyfilepath = optarg;
			break;
		case 'i':
			in = optarg;
			break;
		case 'o':
			out = optarg;
			break;
		default:
			display_help();
		}
	}
	if (optind < argc)
	{
		display_help();
		return 0;
	}

	if (getkey_sign && !encrypt_sign && !decrypt_sign)		//密钥对生成
	{
		//参数检查
		if (privatekeyfilepath == "")
		{
			display_help();
			return 1;
		}
		if (curvename == "")
		{
			curvename = "secp256r1";
		}
		if ((curvename != "secp160r2") && (curvename != "secp192r1") && (curvename != "secp224r1") && (curvename != "secp256r1"))
		{
			cout << "输入的曲线为：" << curvename << endl
				<< "没有所选的曲线！\n"
				<< "支持的曲线：\n"
				<< "secp160r2\tsecp192r1\tsecp224r1\tsecp256r1\n"
				<< endl;
			return 1;
		}
		//获得密钥对
		get_key(curvename.c_str(), privatekey, public_x, public_y);
		cout << "私钥:\n"
			<< privatekey
			<< endl
			<< "公钥:\n"
			<< public_x << endl << public_y
			<< endl;
		//保存密钥对
		if (publickeyfilepath == "")
		{
			publickeyfilepath = privatekeyfilepath + ".pub";
		}
		privatekeyfilepath = privatekeyfilepath + ".key";
		ofstream outprivatekeyfile(privatekeyfilepath);
		ofstream outpublickeyfile(publickeyfilepath);
		outprivatekeyfile << curvename << endl << privatekey;
		outprivatekeyfile.close();
		outpublickeyfile << curvename << endl << public_x << endl << public_y;
		outpublickeyfile.close();
		return 0;
	}
	else if (!getkey_sign && encrypt_sign && !decrypt_sign)	//加密
	{
		//参数检查
		if ((publickeyfilepath == "") || (in == ""))
		{
			display_help();
			return 1;
		}
		//读取文件
		vector<unsigned char> info;
		ifstream publickeyfile(publickeyfilepath);
		if (!publickeyfile)
		{
			cout << "打开公钥文件错误！" << endl;
			return 1;
		}
		else
		{
			getline(publickeyfile, curvename);
			publickeyfile.getline(public_x, 65);
			publickeyfile.getline(public_y, 65);
			publickeyfile.close();
		}
#if defined _MSC_VER
		HANDLE infile = CreateFile(StringToWstring(in).c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		unsigned long long infilesize = GetFileSize(infile, NULL);
		HANDLE infilemap;
		unsigned char *infomap;
		if (infile == INVALID_HANDLE_VALUE)
		{
			CloseHandle(infile);
			cout << "Can't open " << in << " !";
			return 1;
		}
		else
		{

			infilemap = CreateFileMapping(infile, NULL, PAGE_READONLY, 0, 0, NULL);
			if (infilemap == INVALID_HANDLE_VALUE)
			{
				cout << "Can't create file mapping.Error " << GetLastError() << endl;
				CloseHandle(infile);
				return 1;
			}
		}
#elif defined __GNUC__
		ifstream infile(in, ios::binary);
		unsigned long long infilesize = infile.seekg(0, ios::end).tellg();
		infile.seekg(0, ios::beg);
		if (!infile)
		{
			cout << "打开加密文件错误！" << endl;
			return 1;
		}
#endif
		if (out == "")
		{
			out = in + ".encrypted";
		}
		ofstream outfile(out, ios::binary);
		if (!outfile)
		{
			cout << "Can't save " << out << " !";
			return 1;
	}
		//加密
		unsigned long long blockbytes = 100 * 1024 * 1024;		//100MB
		unsigned long long fileoffset = 0;
		unsigned long long cipherdata_length_byte;
		info.resize(blockbytes);
		auto begin_time = chrono::high_resolution_clock::now();
		//分块处理
		while (fileoffset < infilesize)
		{
			//映射块
			if (infilesize - fileoffset < blockbytes)
			{
				blockbytes = infilesize - fileoffset;
			}
#if defined _MSC_VER
			infomap = (unsigned char*)MapViewOfFile(infilemap, FILE_MAP_READ, (DWORD)(fileoffset >> 32), (DWORD)(fileoffset & 0xFFFFFFFF), blockbytes);
			if (infomap == NULL)
			{
				cout << "Can't map.Error " << GetLastError() << endl;
				return 1;
			}
			ReadFile(infomap, &info[0], blockbytes, NULL, NULL);
			UnmapViewOfFile(infomap);
			unsigned char* cipher = ecc_encrypt(curvename.c_str(), public_x, public_y, &info[0], blockbytes, &cipherdata_length_byte);
#elif defined __GNUC__
			infile.read((char*)&info[0], blockbytes);
			unsigned char* cipher = ecc_encrypt(curvename.c_str(), public_x, public_y, &info[0], blockbytes, &cipherdata_length_byte);
#endif
			//保存文件
			outfile.write((const char*)cipher, cipherdata_length_byte);
			free(cipher);
			fileoffset = fileoffset + blockbytes;
		}
		auto end_time = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - begin_time).count();

		cout << duration << "ms" << endl;
		cout << setiosflags(ios::fixed) << setprecision(3) << (float)(infilesize / duration) * 1000 / 1048576 << "MB/s";
		return 0;
	}
	else if (!getkey_sign && !encrypt_sign && decrypt_sign)	//解密
	{
		//参数检查
		if ((privatekeyfilepath == "") || (in == ""))
		{
			display_help();
			return 1;
		}
		//读取文件
		ifstream privatekeyfile(privatekeyfilepath);
		ifstream infile(in, ios::binary);
		unsigned long long infilesize = infile.seekg(0, ios::end).tellg();
		vector<unsigned char> secret;
		if (!privatekeyfile)
		{
			cout << "打开私钥文件错误！" << endl;
			return 1;
		}
		else
		{
			getline(privatekeyfile, curvename);
			privatekeyfile.getline(privatekey, 65);
			privatekeyfile.close();
		}
		if (!infile)
		{
			cout << "打开加密文件错误！" << endl;
			return 1;
		}
		if (out == "")
		{
			out = in.substr(0, in.length() - 10);
		}
		ofstream outfile(out, ios::binary | ios::trunc);
		if (!outfile)
		{
			cout << "Can't save " << out << " !";
			return 1;
		}
		//解密
		unsigned long long blockbytes = 0;
		unsigned long long fileoffset = 0;
		unsigned long long plaindata_length_byte;
		int flag;
		infile.seekg(0, ios::beg).read((char*)&blockbytes, 8);
		secret.resize(blockbytes);
		auto begin_time = chrono::high_resolution_clock::now();
		//分块处理
		infile.seekg(0, ios::beg);
		while (fileoffset < infilesize)
		{
			if (infilesize - fileoffset < blockbytes)
			{
				blockbytes = infilesize - fileoffset;
			}
			infile.read((char*)&secret[0], blockbytes);
			unsigned char* info = ecc_decrypt(privatekey, &secret[0], blockbytes, &plaindata_length_byte, &flag);
			if (flag == 1)
			{
				cout << "Decrypt failed！" << endl;
				return 1;
			}
			//保存文件
			outfile.write((const char*)info, plaindata_length_byte);
			free(info);
			fileoffset = fileoffset + blockbytes;
		}
		auto end_time = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - begin_time).count();
		cout << duration << "ms" << endl;
		cout << setiosflags(ios::fixed) << setprecision(3) << ((float)outfile.tellp() / duration) * 1000 / 1048576 << "MB/s";
		return 0;
	}
	else
	{
		display_help();
		return 1;
	}
}
