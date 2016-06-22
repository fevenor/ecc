#include "basefuncdll.h"
#define EXPORTING_DLL

using namespace std;

int gkey(char const *curve, char *privatekey, char *public_x, char *public_y)
{
	return get_key(curve, privatekey, public_x, public_y);
}

extern _declspec(dllexport) int whetherpoc(char const *curve, char const *public_x, char const *public_y)
{
	return wpoc(curve, public_x, public_y);
}

unsigned char* textencrypt(char const *curve, char const *pub_x, char const *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte)
{
	return ecc_encrypt(curve, pub_x, pub_y, info, info_length_byte, cipherdata_length_byte);
}

int fileencrypt(char const *curve, char const *pub_x, char const *pub_y, char const *in, char const *out)
{

	//��ȡ�ļ�
	vector<unsigned char> info;
	ifstream infile(in, ios::binary);
	unsigned long long infilesize = infile.seekg(0, ios::end).tellg();
	infile.seekg(0, ios::beg);
	if (!infile)
	{
		//���ļ�����!
		return 1;
	}
	if (wpoc(curve, pub_x, pub_y))
	{
		//��Կ����!
		return 3;
	}
	ofstream outfile(out, ios::binary);
	if (!outfile)
	{
		//�����ļ�����!
		return 2;
	}
	//����
	unsigned long long blockbytes = 100 * 1024 * 1024;		//100MB
	unsigned long long fileoffset = 0;
	unsigned long long cipherdata_length_byte;
	info.resize(blockbytes);
	//�ֿ鴦��
	while (fileoffset < infilesize)
	{
		//ӳ���
		if (infilesize - fileoffset < blockbytes)
		{
			blockbytes = infilesize - fileoffset;
		}
		infile.read((char*)&info[0], blockbytes);
		unsigned char* cipher = ecc_encrypt(curve, pub_x, pub_y, &info[0], blockbytes, &cipherdata_length_byte);
		//�����ļ�
		outfile.write((const char*)cipher, cipherdata_length_byte);
		free(cipher);
		fileoffset = fileoffset + blockbytes;
	}
	return 0;
}

unsigned char* textdecrypt(char const *key, unsigned char *secret, unsigned long long cipherdata_length_byte, unsigned long long *plaindata_length_byte, int *flag)
{

	return ecc_decrypt(key, secret, cipherdata_length_byte, plaindata_length_byte, flag);
}

int filedecrypt(char const *key, char const *in, char const *out)
{
	//��ȡ�ļ�
	ifstream infile(in, ios::binary);
	unsigned long long infilesize = infile.seekg(0, ios::end).tellg();
	vector<unsigned char> secret;

	if (!infile)
	{
		//�򿪼����ļ�����!
		return 1;
	}

	ofstream outfile(out, ios::binary | ios::trunc);
	if (!outfile)
	{
		//�����ļ�����!
		return 2;
	}
	//����
	unsigned long long blockbytes = 0;
	unsigned long long fileoffset = 0;
	unsigned long long plaindata_length_byte;
	int flag;
	infile.seekg(0, ios::beg).read((char*)&blockbytes, 8);
	secret.resize(blockbytes);
	//�ֿ鴦��
	infile.seekg(0, ios::beg);
	while (fileoffset < infilesize)
	{
		if (infilesize - fileoffset < blockbytes)
		{
			blockbytes = infilesize - fileoffset;
		}
		infile.read((char*)&secret[0], blockbytes);
		unsigned char* info = ecc_decrypt(key, &secret[0], blockbytes, &plaindata_length_byte, &flag);
		if (flag == 1)
		{
			//Decrypt failed!
			return 3;
		}
		if (flag == 2)
		{
			//˽Կ����!
			return 4;
		}
		//�����ļ�
		outfile.write((const char*)info, plaindata_length_byte);
		free(info);
		fileoffset = fileoffset + blockbytes;
	}
	return 0;
}