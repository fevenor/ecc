#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>

#if defined _MSC_VER								//判断visual studio环境
#include "getopt.h"
#pragma comment(lib,"..\\..\\ecc\\x64\\Release\\basefunc.lib")
#elif defined __GNUC__
#include <getopt.h>
#include "../basefunc/basefunc.h"
#endif

using namespace std;
#if defined _MSC_VER
extern "C" _declspec(dllimport) int get_key(const char *curve, char *privatekey, char *public_x, char *public_y);
extern "C" _declspec(dllimport) unsigned char* ecc_encrypt(const char *curve, const char *pub_x, const char *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte);
extern "C" _declspec(dllimport) unsigned char* ecc_decrypt(const char *key, unsigned char *secret, unsigned long long cipherdata_length_byte, unsigned long long *plaindata_length_byte, int *flag);
#endif

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
		ifstream publickeyfile(publickeyfilepath);
		ifstream infile(in, ios::binary);
		vector<unsigned char> info;
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
		if (!infile)
		{
			cout << "打开待加密文件错误！" << endl;
			return 1;
		}
		else
		{
			info.resize(infile.seekg(0, std::ios::end).tellg());
			infile.seekg(0, std::ios::beg).read((char*)&info[0], static_cast<std::streamsize>(info.size()));
			infile.close();
		}
		//加密
		unsigned long long cipherdata_length_byte;
		auto begin_time = chrono::high_resolution_clock::now();
		unsigned char* cipher = ecc_encrypt(curvename.c_str(), public_x, public_y, &info[0], info.size(), &cipherdata_length_byte);
		auto end_time = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - begin_time).count();
		//保存文件
		if (out == "")
		{
			out = in + ".encrypted";
		}
		ofstream outfile(out, ios::binary);
		outfile.write((char*)cipher, cipherdata_length_byte);
		outfile.close();
		cout << duration << "ms" << endl;
		cout << setiosflags(ios::fixed) << setprecision(3) << (float)(info.size() / duration) * 1000 / 1048576 << "MB/s";
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
		else
		{
			secret.resize(infile.seekg(0, std::ios::end).tellg());
			infile.seekg(0, std::ios::beg).read((char*)&secret[0], static_cast<std::streamsize>(secret.size()));
			infile.close();
		}
		//解密
		unsigned long long plaindata_length_byte;
		int flag;
		auto begin_time = chrono::high_resolution_clock::now();
		unsigned char* info = ecc_decrypt(privatekey, &secret[0], secret.size(), &plaindata_length_byte, &flag);
		auto end_time = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - begin_time).count();
		if (flag == 0)
		{
			//保存文件
			if (out == "")
			{
				out = in.substr(0, in.length() - 10);
			}
			ofstream outfile(out, ios::binary);
			outfile.write((char*)info, plaindata_length_byte);
			outfile.close();
			cout << duration << "ms" << endl;
			cout << setiosflags(ios::fixed) << setprecision(3) << (float)(plaindata_length_byte / duration) * 1000 / 1048576 << "MB/s";
		}
		else
		{
			cout << "私钥文件不匹配或加密数据已损坏！" << endl;
		}

		return 0;
	}
	else
	{
		display_help();
		return 1;
	}
}
