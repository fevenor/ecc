#include "basefunc.h"
#if defined _MSC_VER								//判断visual studio环境
#include "windows.h"
#include "process.h"
#endif

//互斥锁
HANDLE mutex = NULL;

//线程信号
HANDLE *threadsignal;

//线程参数
typedef struct
{
	int threadnum;
	int blocknum;
	int blocklength_byte;
	enum curve_name ecname;
	group *c;
	af_p *pubp;
	unsigned char *(*plaindata);
	unsigned char *(*cipherdata);
}threadarg;

//加密线程
void encrypt_thread(threadarg *t)
{
	int w = 8;
	int i, j, length_diff, partlength;
	af_p *m = af_p_inits();
	af_p *p = af_p_inits();
	af_p *e = af_p_inits();
	af_p *c = af_p_inits();
	mpz_t k;
	mpz_init(k);
	for (i = 0; i < t->blocknum; i++)
	{
		//获得随机数
		//WaitForSingleObject(mutex, INFINITE);
		get_rand(t->c->length, k);
		//ReleaseMutex(mutex);
		mpz_mod(k, k, t->c->n);
		//计算P=kPB
		scalar_mul_w(w, k, t->pubp, p, t->c);
		//计算E=kG
		scalar_mul_c(k, e, t->ecname);
		//将明文转化为点
		mpz_import(m->x, t->blocklength_byte / 2, 1, sizeof(unsigned char), 0, 0, t->plaindata[i]);
		mpz_import(m->y, t->blocklength_byte / 2, 1, sizeof(unsigned char), 0, 0, t->plaindata[i] + (t->blocklength_byte / 2));
		//用P加密明文
		mpz_mul(c->x, m->x, p->x);
		mpz_add(c->x, c->x, p->y);
		mpz_mod(c->x, c->x, t->c->p);
		mpz_mul(c->y, m->y, p->y);
		mpz_add(c->y, c->y, p->x);
		mpz_mod(c->y, c->y, t->c->p);
		//对密文数据类型转换
		partlength = t->c->length / 8;
		//E的x值
		length_diff = (t->c->length / 4 - mpz_sizeinbase(e->x, 16)) / 2;
		for (j = 0; j < length_diff; j++)
		{
			t->cipherdata[i][j] = 0x00;
		}
		mpz_export(t->cipherdata[i] + length_diff, NULL, 1, sizeof(unsigned char), 0, 0, e->x);
		//E的y值
		length_diff = (t->c->length / 4 - mpz_sizeinbase(e->y, 16)) / 2;
		for (j = 0; j < length_diff; j++)
		{
			t->cipherdata[i][j + partlength] = 0x00;
		}
		mpz_export(t->cipherdata[i] + partlength + length_diff, NULL, 1, sizeof(unsigned char), 0, 0, e->y);
		//C的x值
		length_diff = (t->c->length / 4 - mpz_sizeinbase(c->x, 16)) / 2;
		for (j = 0; j < length_diff; j++)
		{
			t->cipherdata[i][j + partlength * 2] = 0x00;
		}
		mpz_export(t->cipherdata[i] + partlength * 2 + length_diff, NULL, 1, sizeof(unsigned char), 0, 0, c->x);
		//C的y值
		length_diff = (t->c->length / 4 - mpz_sizeinbase(c->y, 16)) / 2;
		for (j = 0; j < length_diff; j++)
		{
			t->cipherdata[i][j + partlength * 3] = 0x00;
		}
		mpz_export(t->cipherdata[i] + partlength * 3 + length_diff, NULL, 1, sizeof(unsigned char), 0, 0, c->y);
	}

	//释放内存
	af_p_clears(m);
	af_p_clears(p);
	af_p_clears(e);
	af_p_clears(c);
	mpz_clear(k);
}

//加密模块
unsigned char* encrypt(char *curve, char *pub_x, char *pub_y, unsigned char *info, unsigned long long info_length_byte, unsigned long long *cipherdata_length_byte)
{
	int i, j, blocknum, info_length_diff_byte, blocklength_byte, secret_blocklength_byte, threadnum, bnft, lastbnft;
	mpz_t fillk;
	af_p *pubp = af_p_inits();
	group *c = group_inits();
	SYSTEM_INFO siSysInfo;
	unsigned char *(*plaindata);
	unsigned char *(*cipherdata);
	unsigned char *plain;
	unsigned char *secret;
	threadarg *(*t);
	enum curve_name ecname;
	//获得曲线参数
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
	//导入原始数据
	blocklength_byte = 2 * (c->length / 8 - 1);
	blocknum = (info_length_byte + 8 - 1) / (blocklength_byte)+1;		//分块数
	plain = malloc(sizeof(unsigned char)*(blocknum*blocklength_byte));
	info_length_diff_byte = blocknum*blocklength_byte - (info_length_byte + 8);
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
	//为密文分配内存
	secret_blocklength_byte = 4 * c->length / 8;
	secret = malloc(sizeof(unsigned char)*(blocknum*secret_blocklength_byte + 1));
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
	for (i = 0; i < blocknum; i++)
	{
		cipherdata[i] = secret + 1 + secret_blocklength_byte*i;
	}
	*cipherdata_length_byte = secret_blocklength_byte*blocknum + 1;
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
			t[i]->ecname = ecname;
			t[i]->c = c;
			t[i]->pubp = pubp;
			t[i]->plaindata = malloc(sizeof(unsigned char *)*bnft);
			t[i]->cipherdata = malloc(sizeof(unsigned char *)*bnft);
			for (j = 0; j < bnft; j++)
			{
				t[i]->plaindata[j] = plaindata[i*bnft + j];
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
			t[threadnum]->ecname = ecname;
			t[threadnum]->c = c;
			t[threadnum]->pubp = pubp;
			t[threadnum]->plaindata = malloc(sizeof(unsigned char *)*lastbnft);
			t[threadnum]->cipherdata = malloc(sizeof(unsigned char *)*lastbnft);
			for (j = 0; j < lastbnft; j++)
			{
				t[threadnum]->plaindata[j] = plaindata[threadnum*bnft + j];
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
		t[0]->ecname = ecname;
		t[0]->c = c;
		t[0]->pubp = pubp;
		t[0]->plaindata = malloc(sizeof(unsigned char *)*blocknum);
		t[0]->plaindata[0] = plaindata[0];
		t[0]->cipherdata = malloc(sizeof(unsigned char *)*blocknum);
		t[0]->cipherdata[0] = cipherdata[0];
		for (j = 0; j < blocknum; j++)
		{
			t[0]->plaindata[j] = plaindata[j];
			t[0]->cipherdata[j] = cipherdata[j];
		}
		encrypt_thread(t[0]);
		free(t[0]->plaindata);
		free(t[0]->cipherdata);
		free(t[0]);
	}

	//释放内存
	group_clears(c);
	af_p_clears(pubp);
	free(plain);
	free(plaindata);
	free(cipherdata);
	free(t);
	return secret;
}
