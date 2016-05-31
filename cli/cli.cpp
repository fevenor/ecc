#include <iostream>
#include <fstream>
#include <string>
#include<vector>

#if defined _MSC_VER								//�ж�visual studio����
#include "getopt.h"
#pragma comment(lib,"..\\..\\ecc\\x64\\Release\\basefunc.lib")
#endif

using namespace std;

extern "C" _declspec(dllimport) int get_key(const char *curve, char *privatekey, char *public_x, char *public_y);
extern "C" _declspec(dllimport) unsigned char* encrypt(const char *curve, const char *pub_x, const char *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte);
extern "C" _declspec(dllimport) unsigned char* decrypt(const char *key, unsigned char *secret, unsigned long long cipherdata_length_byte, unsigned long long *plaindata_length_byte);

void display_help()
{
	cout << endl
		<< "�÷���\n"
		<< "��Կ�����ɣ�\n"
		<< " ecc -g|--getkey [-n|--name curve_name] -k|--privatekey privatekey_file [-p|--publickey publickey_file]\n"
		<< "���ܣ�\n"
		<< " ecc -e|--encrypt -p|--publickey publickey_file -i|--in file [-o|--out encrypted_file]\n"
		<< "���ܣ�\n"
		<< " ecc -d|--decrypt -k|--privatekey privatekey_file -i|--in encrypted_file [-o|--out decrypted_file]\n"
		<< endl
		<< "ѡ�\n"
		<< "-g, --getkey\t\t��Կ������\n"
		<< "-n, --name\t\t������\n"
		<< "-e, --encrypt\t\t����\n"
		<< "-d, --decrypt\t\t����\n"
		<< "-k, --privatekey\t˽Կ�ļ�\n"
		<< "-p, --publickey\t\t��Կ�ļ�\n"
		<< "-i, --in\t\t�����ļ�\n"
		<< "-o, --out\t\t����ļ�\n"
		<< endl
		<< "֧�ֵ����ߣ�\n"
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
	char* const short_options = "gn:edp:k:i:o:";
	struct option long_options[] = {
		{ "getkey", 0, NULL, 'g' },
		{ "encrypt", 0, NULL, 'e' },
		{ "decrypt", 0, NULL, 'd' },
		{ "name", 1, NULL, 'n' },
		{ "privatekey", 1, NULL, 'k' },
		{ "publickey", 1, NULL, 'p' },
		{ "in", 1, NULL, 'i' },
		{ "out", 1, NULL, 'o' },
		{ 0, 0, 0, 0 },
	};
	int opt;
	int option_index = 0;
	bool getkey_sign = false, encrypt_sign = false, decrypt_sign = false;
	string curvename, privatekeyfilepath, publickeyfilepath, in, out;
	char privatekey[65], public_x[65], public_y[65];

	while ((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)	//��ȡѡ��
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

	if (getkey_sign && !encrypt_sign && !decrypt_sign)		//��Կ������
	{
		//�������
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
			cout << "���������Ϊ��" << curvename << endl
				<< "û����ѡ�����ߣ�\n"
				<< "֧�ֵ����ߣ�\n"
				<< "secp160r2\tsecp192r1\tsecp224r1\tsecp256r1\n"
				<< endl;
			return 1;
		}
		//�����Կ��
		get_key(curvename.c_str(), privatekey, public_x, public_y);
		cout << "˽Կ:\n"
			<< privatekey
			<< endl
			<< "��Կ:\n"
			<< public_x << endl << public_y
			<< endl;
		//������Կ��
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
	else if (!getkey_sign && encrypt_sign && !decrypt_sign)	//����
	{
		//�������
		if ((publickeyfilepath == "") || (in == ""))
		{
			display_help();
			return 1;
		}
		//��ȡ�ļ�
		ifstream publickeyfile(publickeyfilepath);
		ifstream infile(in, ios::binary);
		vector<unsigned char> info;
		if (!publickeyfile)
		{
			cout << "�򿪹�Կ�ļ�����" << endl;
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
			cout << "�򿪴������ļ�����" << endl;
			return 1;
		}
		else
		{
			info.resize(infile.seekg(0, std::ios::end).tellg());
			infile.seekg(0, std::ios::beg).read((char*)&info[0], static_cast<std::streamsize>(info.size()));
			infile.close();
		}
		//����
		unsigned long long cipherdata_length_byte;
		unsigned char* cipher = encrypt(curvename.c_str(), public_x, public_y, &info[0], info.size(), &cipherdata_length_byte);
		//�����ļ�
		if (out == "")
		{
			out = in + ".encrypted";
		}
		ofstream outfile(out, ios::binary);
		outfile.write((char*)cipher, cipherdata_length_byte);
		outfile.close();
		return 0;
	}
	else if (!getkey_sign && !encrypt_sign && decrypt_sign)	//����
	{
		//�������
		if ((privatekeyfilepath == "") || (in == ""))
		{
			display_help();
			return 1;
		}
		//��ȡ�ļ�
		ifstream privatekeyfile(privatekeyfilepath);
		ifstream infile(in, ios::binary);
		vector<unsigned char> secret;
		if (!privatekeyfile)
		{
			cout << "��˽Կ�ļ�����" << endl;
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
			cout << "�򿪼����ļ�����" << endl;
			return 1;
		}
		else
		{
			secret.resize(infile.seekg(0, std::ios::end).tellg());
			infile.seekg(0, std::ios::beg).read((char*)&secret[0], static_cast<std::streamsize>(secret.size()));
			infile.close();
		}
		//����
		unsigned long long plaindata_length_byte;
		unsigned char* info = decrypt(privatekey, &secret[0], secret.size(), &plaindata_length_byte);
		//�����ļ�
		if (out == "")
		{
			out = in.substr(0, in.length() - 10);
		}
		ofstream outfile(out, ios::binary);
		outfile.write((char*)info, plaindata_length_byte);
		outfile.close();
		return 0;
	}
	else
	{
		display_help();
		return 1;
	}
}