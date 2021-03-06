﻿#include "basefunc.h"

void get_rand(int length, mpz_t k)
{
#if defined _MSC_VER
	HCRYPTPROV hProvider = 0;
	BYTE *buf = malloc(sizeof(BYTE) * length / 8);
	//CryptAcquireContextW(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
	CryptGenRandom(hProvider, length / 8, buf);
	CryptReleaseContext(hProvider, 0);
#elif defined __GNUC__
	FILE *f;
	unsigned char *buf = malloc(sizeof(unsigned char) * length / 8);
	f = fopen("/dev/urandom", "r");
	fread(buf, sizeof(unsigned char) * length / 8, 1, f);
	fclose(f);
#endif
	mpz_import(k, length / 8, 1, sizeof(buf[0]), 0, 0, buf);
	free(buf);
}
