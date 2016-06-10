#include "basefunc.h"

//互斥锁
#if defined _MSC_VER
static HANDLE mutex = NULL;
#elif defined __GNUC__
pthread_mutex_t mutex;
#endif

//线程信号
#if defined _MSC_VER
static HANDLE *threadsignal;
#elif defined __GNUC__
pthread_t *threadsignal;
#endif

//曲线参数
static group *c;

//线程参数
typedef struct
{
	int threadnum;
	int blocknum;
	int blocklength_byte;
	af_p *p;
	mpz_t inverse_x;
	unsigned char *(*plaindata);
	unsigned char *(*cipherdata);
}threadarg;

//解密线程
void decrypt_thread(threadarg *t)
{
	int i, j, length_diff;
	mpz_t m;
	mpz_init(m);
	unsigned char *temp = malloc(sizeof(unsigned char)*(c->length / 8 - 1));
	for (i = 0; i < t->blocknum; i++)
	{
		//将对密文数据类型转换
		mpz_import(m, t->blocklength_byte, 1, sizeof(unsigned char), 0, 0, t->cipherdata[i]);
		//用P解密
		mpz_sub(m, m, t->p->y);
		mpz_mod(m, m, c->p);
		mpz_mul(m, m, t->inverse_x);
		mpz_mod(m, m, c->p);
		//对明文数据类型转换
		length_diff = (c->length / 4 - 2 - (int)mpz_sizeinbase(m, 16)) / 2;
		for (j = 0; j < length_diff; j++)
		{
			temp[j] = 0x00;
		}
		mpz_export(temp + length_diff, NULL, 1, sizeof(unsigned char), 0, 0, m);
		//提取真实信息
		int salt_location = (int)temp[0] % (c->length / 16);
		memcpy(t->plaindata[i], temp + 1, salt_location);
		memcpy(t->plaindata[i] + salt_location, temp + 1 + salt_location + c->length / 16 - 2, c->length / 16 - salt_location);
	}

	//释放内存
	mpz_clear(m);
	free(temp);
}


//解密模块
unsigned char* ecc_decrypt(char const *key, unsigned char *secret, unsigned long long cipherdata_length_byte, unsigned long long *plaindata_length_byte, int *flag)
{
	//SHA1校验
	unsigned char hash1[sha1_len];
	memcpy(hash1, secret + 8, 20);
	unsigned char hash2[sha1_len] = { 0 };
	SHA1(secret + 28, cipherdata_length_byte - 28, hash2);

	if (!memcmp(hash1, hash2, 20))
	{
		*flag = 0;
	}
	else
	{
		*flag = 1;
		*plaindata_length_byte = 0;
		unsigned char *plain = calloc(1, sizeof(unsigned char));
		return plain;
	}

	int w = 8;
	int i, j, blocknum, blocklength_byte, plain_blocklength_byte, threadnum, bnft, lastbnft;
	mpz_t k, inverse_x;
	c = group_inits();
	unsigned char *(*plaindata);
	unsigned char *(*cipherdata);
	unsigned char *plain;
	enum curve_name ecname;
	af_p *p = af_p_inits();
	af_p *e = af_p_inits();
	threadarg *(*t);
	//获得曲线参数
	if (secret[28] == 0x00)
	{
		ecname = 0;
	}
	else if (secret[28] == 0x01)
	{
		ecname = 1;
	}
	else if (secret[28] == 0x02)
	{
		ecname = 2;
	}
	else if (secret[28] == 0x03)
	{
		ecname = 3;
	}
	get_curve_parameters(ecname, c);
	//导入私钥
	mpz_init_set_str(k, key, 16);
	//导入E点
	mpz_import(e->x, c->length / 8, 1, sizeof(unsigned char), 0, 0, secret + 29);
	mpz_import(e->y, c->length / 8, 1, sizeof(unsigned char), 0, 0, secret + 29 + c->length / 8);
	//计算P=nB(kG)=k(nBG)=kPB
	scalar_mul_w(w, k, e, p, c);
	mpz_init(inverse_x);
	mpz_invert(inverse_x, p->x, c->p);
	//导入密文
	cipherdata_length_byte = cipherdata_length_byte - 1 - c->length / 4;
	blocklength_byte = c->length / 8;						//块大小
	blocknum = (int)(cipherdata_length_byte / blocklength_byte);		//分块数
	cipherdata = malloc(sizeof(unsigned char *)*blocknum);
	for (i = 0; i < blocknum; i++)
	{
		cipherdata[i] = secret + 29 + c->length / 4 + blocklength_byte * i;
	}
	//为明文分配内存
	plain_blocklength_byte = c->length / 16;
	plain = malloc(sizeof(unsigned char)*blocknum*plain_blocklength_byte);
	plaindata = malloc(sizeof(unsigned char *)*blocknum);
	for (i = 0; i < blocknum; i++)
	{
		plaindata[i] = plain + plain_blocklength_byte*i;
	}
	//多线程解密处理
#if defined _MSC_VER
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);		//获得系统信息
	threadnum = siSysInfo.dwNumberOfProcessors;		//根据逻辑处理器个数确定主要线程数
#elif defined __GNUC__
	threadnum = sysconf(_SC_NPROCESSORS_ONLN);
#endif
	if (blocknum >= threadnum)						//如果需要处理的块数足够多，多线程处理
	{
		bnft = blocknum / threadnum;			//每个线程处理块数
		lastbnft = blocknum % threadnum;		//可能多余的块数
		if (lastbnft != 0)
		{
			t = malloc(sizeof(threadarg*)*(threadnum + 1));
#if defined _MSC_VER
			threadsignal = malloc(sizeof(HANDLE) * (threadnum + 1));
#elif defined __GNUC__
			threadsignal = malloc(sizeof(pthread_t) * (threadnum + 1));
#endif
		}
		else
		{
			t = malloc(sizeof(threadarg*)*threadnum);
#if defined _MSC_VER
			threadsignal = malloc(sizeof(HANDLE) * threadnum);
#elif defined __GNUC__
			threadsignal = malloc(sizeof(pthread_t) * threadnum);
#endif
		}
#if defined _MSC_VER
		mutex = CreateMutex(NULL, FALSE, NULL);
#elif defined __GNUC__
		pthread_mutex_init(&mutex, NULL);
#endif
		for (i = 0; i < threadnum; i++)
		{
			t[i] = malloc(sizeof(threadarg));
			t[i]->threadnum = i;
			t[i]->blocklength_byte = blocklength_byte;
			t[i]->blocknum = bnft;
			t[i]->p = p;
			mpz_init_set(t[i]->inverse_x, inverse_x);
			t[i]->plaindata = malloc(sizeof(unsigned char *)*bnft);
			t[i]->cipherdata = malloc(sizeof(unsigned char *)*bnft);
			for (j = 0; j < bnft; j++)
			{
				t[i]->plaindata[j] = plaindata[i*bnft + j];
				t[i]->cipherdata[j] = cipherdata[i*bnft + j];
			}
#if defined _MSC_VER
			threadsignal[i] = _beginthread(decrypt_thread, 0, t[i]);
#elif defined __GNUC__
			pthread_create(&threadsignal[i], NULL, (void *)decrypt_thread, t[i]);
#endif
		}

		if (lastbnft != 0)
		{
			t[threadnum] = malloc(sizeof(threadarg));
			t[threadnum]->threadnum = threadnum;
			t[threadnum]->blocklength_byte = blocklength_byte;
			t[threadnum]->blocknum = lastbnft;
			t[threadnum]->p = p;
			mpz_init_set(t[threadnum]->inverse_x, inverse_x);
			t[threadnum]->plaindata = malloc(sizeof(unsigned char *)*lastbnft);
			t[threadnum]->cipherdata = malloc(sizeof(unsigned char *)*lastbnft);
			for (j = 0; j < lastbnft; j++)
			{
				t[threadnum]->plaindata[j] = plaindata[threadnum*bnft + j];
				t[threadnum]->cipherdata[j] = cipherdata[threadnum*bnft + j];
			}
#if defined _MSC_VER
			threadsignal[threadnum] = _beginthread(decrypt_thread, 0, t[threadnum]);
#elif defined __GNUC__
			pthread_create(&threadsignal[threadnum], NULL, (void *)decrypt_thread, t[threadnum]);
#endif
		}

		if (lastbnft == 0)
		{
#if defined _MSC_VER
			WaitForMultipleObjects(threadnum, threadsignal, true, INFINITE);
#endif
			for (i = 0; i < threadnum; i++)
			{
#if defined __GNUC__
				pthread_join(threadsignal[i], NULL);
#endif
				free(t[i]->plaindata);
				mpz_clear(t[i]->inverse_x);
				free(t[i]->cipherdata);
				free(t[i]);
			}
		}
		else
		{
#if defined _MSC_VER
			WaitForMultipleObjects(threadnum + 1, threadsignal, true, INFINITE);
#endif
			for (i = 0; i < threadnum + 1; i++)
			{
#if defined __GNUC__
				pthread_join(threadsignal[i], NULL);
#endif
				free(t[i]->plaindata);
				mpz_clear(t[i]->inverse_x);
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
		mpz_init_set(t[0]->inverse_x, inverse_x);
		t[0]->plaindata = malloc(sizeof(unsigned char *)*blocknum);
		t[0]->cipherdata = malloc(sizeof(unsigned char *)*blocknum);
		for (j = 0; j < blocknum; j++)
		{
			t[0]->plaindata[j] = plaindata[j];
			t[0]->cipherdata[j] = cipherdata[j];
		}
		decrypt_thread(t[0]);
		free(t[0]->plaindata);
		mpz_clear(t[0]->inverse_x);
		free(t[0]->cipherdata);
		free(t[0]);
	}

	memcpy(plaindata_length_byte, plain, 8);
	memmove(plain, plain + 8, sizeof(unsigned char)*(*plaindata_length_byte));

	//释放内存
	mpz_clear(k);
	mpz_clear(inverse_x);
	group_clears(c);
	af_p_clears(p);
	af_p_clears(e);
	free(plaindata);
	free(cipherdata);
	return plain;
}
