#include "basefunc.h"

//互斥锁
static HANDLE mutex = NULL;

//线程信号
static HANDLE *threadsignal;

//曲线参数
static group *c;

//线程参数
typedef struct
{
	int threadnum;
	int blocknum;
	int blocklength_byte;
	af_p *p;
	unsigned char *(*plaindata);
	unsigned char *(*saltdata);
	unsigned char *(*cipherdata);
}threadarg;

//加密线程
void encrypt_thread(threadarg *t)
{

	int i, j, length_diff;
	mpz_t m;
	unsigned char *temp = malloc(sizeof(unsigned char)*(c->length / 8 - 1));
	mpz_init(m);
	for (i = 0; i < t->blocknum; i++)
	{
		//对明文加盐
		memcpy(temp, t->plaindata[i], t->blocklength_byte);
		memcpy(temp + (c->length / 16), t->saltdata[i], t->blocklength_byte - 1);
		//对明文数据类型转换
		mpz_import(m, c->length / 8 - 1, 1, sizeof(unsigned char), 0, 0, temp);
		//用P加密明文
		mpz_mul(m, m, t->p->x);
		mpz_add(m, m, t->p->y);
		mpz_mod(m, m, c->p);
		//对密文数据类型转换
		length_diff = (c->length / 4 - (int)mpz_sizeinbase(m, 16)) / 2;
		for (j = 0; j < length_diff; j++)
		{
			t->cipherdata[i][j] = 0x00;
		}
		mpz_export(t->cipherdata[i] + length_diff, NULL, 1, sizeof(unsigned char), 0, 0, m);
	}
	//释放内存
	mpz_clear(m);
	free(temp);
}

//加密模块
unsigned char* encrypt(char *curve, char *pub_x, char *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte)
{
	int i, j, blocknum, info_length_diff_byte, blocklength_byte, secret_blocklength_byte, length_diff, threadnum, bnft, lastbnft;
	int w = 8;
	mpz_t fillk;
	af_p *pubp = af_p_inits();
	af_p *p = af_p_inits();
	af_p *e = af_p_inits();
	SYSTEM_INFO siSysInfo;
	unsigned char *(*plaindata);
	unsigned char *(*saltdata);
	unsigned char *(*cipherdata);
	unsigned char *plain;
	unsigned char *salt;
	unsigned char *secret;
	threadarg *(*t);
	enum curve_name ecname;
	mpz_t k;
	HCRYPTPROV salt_p = 0;
	mpz_init(k);
	//获得曲线参数
	c = group_inits();
	if (strcmp(curve, "secp160r2") == 0)
	{
		ecname = 0;
	}
	else if (strcmp(curve, "secp192r1") == 0)
	{
		ecname = 1;
	}
	else if (strcmp(curve, "secp224r1") == 0)
	{
		ecname = 2;
	}
	else if (strcmp(curve, "secp256r1") == 0)
	{
		ecname = 3;
	}
	get_curve_parameters(ecname, c);
	//导入公钥
	mpz_set_str(pubp->x, pub_x, 16);
	mpz_set_str(pubp->y, pub_y, 16);
	//计算会话密钥
	//获得随机数
	get_rand(c->length, k);
	mpz_mod(k, k, c->n);
	//计算P=kPB
	scalar_mul_w(w, k, pubp, p, c);
	//计算E=kG
	scalar_mul_c(k, e, ecname);
	//导入原始数据
	blocklength_byte = (c->length / 16);
	blocknum = (int)((info_length_byte + 8 - 1) / (blocklength_byte)+1);		//分块数,8字节为原始数据长度信息
	plain = malloc(sizeof(unsigned char)*(blocknum*blocklength_byte));
	info_length_diff_byte = (int)(blocknum*(unsigned long long)blocklength_byte - (info_length_byte + 8));
	memcpy(plain, &info_length_byte, 8);
	if (info_length_diff_byte == 0)
	{
		memcpy(plain + 8, info, info_length_byte);
	}
	else
	{
		mpz_init(fillk);
		memcpy(plain + 8, info, info_length_byte);
		get_rand(info_length_diff_byte * 8, fillk);
		mpz_export(plain + 8 + info_length_byte, NULL, 1, sizeof(unsigned char), 0, 0, fillk);
		mpz_clear(fillk);
	}
	plaindata = malloc(sizeof(unsigned char *)*blocknum);
	for (i = 0; i < blocknum; i++)
	{
		plaindata[i] = plain + blocklength_byte*i;
	}
	//导入随机数据
	salt = malloc(sizeof(unsigned char)*(blocknum*(blocklength_byte-1)));
	CryptAcquireContext(&salt_p, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
	CryptGenRandom(salt_p, blocknum*(blocklength_byte - 1), salt);
	CryptReleaseContext(salt_p, 0);
	saltdata = malloc(sizeof(unsigned char *)*blocknum);
	for (i = 0; i < blocknum; i++)
	{
		saltdata[i] = salt + (blocklength_byte-1)*i;
	}
	//为密文分配内存
	secret_blocklength_byte = c->length / 8;
	secret = malloc(sizeof(unsigned char)*(blocknum*secret_blocklength_byte + 1 + c->length / 4));	//1字节的曲线编号,曲线长度/4字节的E点信息
	cipherdata = malloc(sizeof(unsigned char *)*blocknum);
	if (ecname == 0)
	{
		secret[0] = 0x00;
	}
	else if (ecname == 1)
	{
		secret[0] = 0x01;
	}
	else if (ecname == 2)
	{
		secret[0] = 0x02;
	}
	else if (ecname == 3)
	{
		secret[0] = 0x03;
	}
	//导出E点
	//E的x值
	length_diff = (c->length / 4 - (int)mpz_sizeinbase(e->x, 16)) / 2;
	for (i = 0; i < length_diff; i++)
	{
		secret[1 + i] = 0x00;
	}
	mpz_export(secret + 1 + length_diff, NULL, 1, sizeof(unsigned char), 0, 0, e->x);
	//E的y值
	length_diff = (c->length / 4 - (int)mpz_sizeinbase(e->y, 16)) / 2;
	for (i = 0; i < length_diff; i++)
	{
		secret[1 + i + c->length / 8] = 0x00;
	}
	mpz_export(secret + 1 + c->length / 8 + length_diff, NULL, 1, sizeof(unsigned char), 0, 0, e->y);
	//定义密文块地址
	for (i = 0; i < blocknum; i++)
	{
		cipherdata[i] = secret + 1 + c->length / 4 + secret_blocklength_byte*i;
	}
	*cipherdata_length_byte = blocknum*secret_blocklength_byte + 1 + c->length / 4;
	//多线程加密处理
	GetSystemInfo(&siSysInfo);		//获得系统信息
	threadnum = siSysInfo.dwNumberOfProcessors;		//根据逻辑处理器个数确定主要线程数
	if (blocknum >= threadnum)						//如果需要处理的块数足够多，多线程处理
	{
		bnft = blocknum / threadnum;			//每个线程处理块数
		lastbnft = blocknum % threadnum;		//可能多余的块数
		if (lastbnft != 0)
		{
			t = malloc(sizeof(threadarg*)*(threadnum + 1));
			threadsignal = malloc(sizeof(HANDLE) * (threadnum + 1));
		}
		else
		{
			t = malloc(sizeof(threadarg*)*threadnum);
			threadsignal = malloc(sizeof(HANDLE) * threadnum);
		}
		mutex = CreateMutex(NULL, FALSE, NULL);
		for (i = 0; i < threadnum; i++)
		{
			t[i] = malloc(sizeof(threadarg));
			t[i]->threadnum = i;
			t[i]->blocklength_byte = blocklength_byte;
			t[i]->blocknum = bnft;
			t[i]->p = p;
			t[i]->plaindata = malloc(sizeof(unsigned char *)*bnft);
			t[i]->saltdata = malloc(sizeof(unsigned char *)*bnft);
			t[i]->cipherdata = malloc(sizeof(unsigned char *)*bnft);
			for (j = 0; j < bnft; j++)
			{
				t[i]->plaindata[j] = plaindata[i*bnft + j];
				t[i]->saltdata[j] = saltdata[i*bnft + j];
				t[i]->cipherdata[j] = cipherdata[i*bnft + j];
			}
			threadsignal[i] = _beginthread(encrypt_thread, 0, t[i]);
		}
		if (lastbnft != 0)
		{
			t[threadnum] = malloc(sizeof(threadarg));
			t[threadnum]->threadnum = threadnum;
			t[threadnum]->blocklength_byte = blocklength_byte;
			t[threadnum]->blocknum = lastbnft;
			t[threadnum]->p = p;
			t[threadnum]->plaindata = malloc(sizeof(unsigned char *)*lastbnft);
			t[threadnum]->saltdata = malloc(sizeof(unsigned char *)*lastbnft);
			t[threadnum]->cipherdata = malloc(sizeof(unsigned char *)*lastbnft);
			for (j = 0; j < lastbnft; j++)
			{
				t[threadnum]->plaindata[j] = plaindata[threadnum*bnft + j];
				t[threadnum]->saltdata[j] = saltdata[threadnum*bnft + j];
				t[threadnum]->cipherdata[j] = cipherdata[threadnum*bnft + j];
			}
			threadsignal[threadnum] = _beginthread(encrypt_thread, 0, t[threadnum]);
		}

		if (lastbnft == 0)
		{
			WaitForMultipleObjects(threadnum, threadsignal, true, INFINITE);
			for (i = 0; i < threadnum; i++)
			{
				free(t[i]->plaindata);
				free(t[i]->saltdata);
				free(t[i]->cipherdata);
				free(t[i]);
			}
		}
		else
		{
			WaitForMultipleObjects(threadnum + 1, threadsignal, true, INFINITE);
			for (i = 0; i < threadnum + 1; i++)
			{
				free(t[i]->plaindata);
				free(t[i]->saltdata);
				free(t[i]->cipherdata);
				free(t[i]);
			}

		}
		free(threadsignal);
	}
	else
	{
		t = malloc(sizeof(threadarg*));
		t[0] = malloc(sizeof(threadarg));
		t[0]->threadnum = 0;
		t[0]->blocklength_byte = blocklength_byte;
		t[0]->blocknum = blocknum;
		t[0]->p = p;
		t[0]->plaindata = malloc(sizeof(unsigned char *)*blocknum);
		t[0]->saltdata = malloc(sizeof(unsigned char *)*blocknum);
		t[0]->cipherdata = malloc(sizeof(unsigned char *)*blocknum);
		for (j = 0; j < blocknum; j++)
		{
			t[0]->plaindata[j] = plaindata[j];
			t[0]->saltdata[j] = saltdata[j];
			t[0]->cipherdata[j] = cipherdata[j];
		}
		encrypt_thread(t[0]);
		free(t[0]->plaindata);
		free(t[0]->saltdata);
		free(t[0]->cipherdata);
		free(t[0]);
	}

	//释放内存
	mpz_clear(k);
	group_clears(c);
	af_p_clears(pubp);
	af_p_clears(p);
	af_p_clears(e);
	free(plain);
	free(plaindata);
	free(salt);
	free(saltdata);
	free(cipherdata);
	free(t);
	return secret;
}
