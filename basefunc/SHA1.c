#include "basefunc.h"
#if defined _MSC_VER
void SHA1(unsigned char *info, unsigned long long info_length_byte, unsigned char *hash)
{
	HCRYPTPROV  hProv = NULL;
	HCRYPTHASH  hHash = NULL;
	int hashlen;

	CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash);
	CryptHashData(hHash, info, info_length_byte, 0);
	CryptGetHashParam(hHash, HP_HASHVAL, NULL, &hashlen, 0);
	CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashlen, 0);
	CryptReleaseContext(hProv, 0);
	CryptDestroyHash(hHash);
}
#endif
