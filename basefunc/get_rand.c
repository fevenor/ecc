#include "basefunc.h"
#include "windows.h"

void get_rand(int length, mpz_t k)
{
	HCRYPTPROV hProvider = 0;
	BYTE *buf = malloc(sizeof(BYTE) * length / 8);
	CryptAcquireContextW(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	//CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
	CryptGenRandom(hProvider, length / 8, buf);
	CryptReleaseContext(hProvider, 0);
	mpz_import(k, length / 8, 1, sizeof(buf[0]), 0, 0, buf);
}