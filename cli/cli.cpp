#include <iostream>
#include <fstream>
#include <string>

#if defined _MSC_VER								//�ж�visual studio����
#include "getopt.h"
#pragma comment(lib,"..\\..\\ecc\\x64\\Release\\basefunc.lib")
#endif

using namespace std;

extern "C" _declspec(dllimport) int get_key(const char *curve, char *privatekey, char *public_x, char *public_y);

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
		return 0;
	}
	opterr = 0;
	char* const short_options = "gn:edp:k:i:o::";
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
	string curvename, privatekeyfile, publickeyfile, in, out;
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
			privatekeyfile = optarg;
			break;
		case 'p':
			publickeyfile = optarg;
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
		if (privatekeyfile == "")
		{
			display_help();
			return 0;
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
			return 0;
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
		if (publickeyfile == "")
		{
			publickeyfile = privatekeyfile + ".pub";
		}
		privatekeyfile = privatekeyfile + ".key";
		ofstream outprivatekeyfile(privatekeyfile);
		ofstream outpublickeyfile(publickeyfile);
		outprivatekeyfile << curvename << endl << privatekey;
		outprivatekeyfile.close();
		outpublickeyfile << curvename << endl << public_x << endl << public_y;
		outpublickeyfile.close();
		return 0;
	}
	else if (!getkey_sign && encrypt_sign && !decrypt_sign)	//����
	{

	}
	else if (!getkey_sign && !encrypt_sign && decrypt_sign)	//����
	{

	}
	else
	{
		display_help();
	}

	return 0;
}