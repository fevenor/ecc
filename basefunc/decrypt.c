#include "basefunc.h"

//互斥锁
static HANDLE mutex = NULL;

//线程信号
static HANDLE *threadsignal;

//线程参数
typedef struct
{
	int threadnum;
	int blocknum;
	int blocklength_byte;
	enum curve_name ecname;
	group *c;
	mpz_t *k;
	unsigned char *(*plaindata);
	unsigned char *(*cipherdata);
}threadarg;

//解密线程
void decrypt_thread(threadarg *t)
{
	int w = 8;
	int i, j, length_diff, partlength;
	mpz_t temp;
	af_p *m = af_p_inits();
	af_p *p = af_p_inits();
	af_p *e = af_p_inits();
	af_p *c = af_p_inits();
	mpz_init(temp);
	for (i = 0; i < t->blocknum; i++)
	{
		//将密文转化为点
		mpz_import(e->x, t->blocklength_byte / 4, 1, sizeof(unsigned char), 0, 0, t->cipherdata[i]);
		mpz_import(e->y, t->blocklength_byte / 4, 1, sizeof(unsigned char), 0, 0, t->cipherdata[i] + (t->blocklength_byte / 4));
		mpz_import(c->x, t->blocklength_byte / 4, 1, sizeof(unsigned char), 0, 0, t->cipherdata[i] + 2 * (t->blocklength_byte / 4));
		mpz_import(c->y, t->blocklength_byte / 4, 1, sizeof(unsigned char), 0, 0, t->cipherdata[i] + 3 * (t->blocklength_byte / 4));
		//计算P=nB(kG)=k(nBG)=kPB
		scalar_mul_w(w, t->k, e, p, t->c);
		//用P解密
		mpz_sub(m->x, c->x, p->y);
		mpz_invert(temp, p->x, t->c->p);
		mpz_mul(m->x, m->x, temp);
		mpz_mod(m->x, m->x, t->c->p);
		mpz_sub(m->y, c->y, p->x);
		mpz_invert(temp, p->y, t->c->p);
		mpz_mul(m->y, m->y, temp);
		mpz_mod(m->y, m->y, t->c->p);
		//对明文数据类型转换
		partlength = t->c->length / 8 - 1;
		//M的x值
		length_diff = (t->c->length / 4 - 2 - (int)mpz_sizeinbase(m->x, 16)) / 2;
		for (j = 0; j < length_diff; j++)
		{
			t->plaindata[i][j] = 0x00;
		}
		mpz_export(t->plaindata[i] + length_diff, NULL, 1, sizeof(unsigned char), 0, 0, m->x);
		//M的y值
		length_diff = (t->c->length / 4 - 2 - (int)mpz_sizeinbase(m->y, 16)) / 2;
		for (j = 0; j < length_diff; j++)
		{
			t->plaindata[i][j + partlength] = 0x00;
		}
		mpz_export(t->plaindata[i] + partlength + length_diff, NULL, 1, sizeof(unsigned char), 0, 0, m->y);
	}

	//释放内存
	af_p_clears(m);
	af_p_clears(p);
	af_p_clears(e);
	af_p_clears(c);
	mpz_clear(temp);
}

//解密模块
unsigned char* decrypt(char *key, unsigned char *secret, unsigned long long cipherdata_length_byte, unsigned long long *plaindata_length_byte)
{
	int i, j, blocknum, blocklength_byte, plain_blocklength_byte, threadnum, bnft, lastbnft;
	mpz_t k;
	group *c = group_inits();
	SYSTEM_INFO siSysInfo;
	unsigned char *(*plaindata);
	unsigned char *(*cipherdata);
	unsigned char *plain;
	//unsigned char *secret;
	threadarg *(*t);
	enum curve_name ecname;
	//获得曲线参数
	if (secret[0] == 0x00)
	{
		ecname = 0;
	}
	else if (secret[0] == 0x01)
	{
		ecname = 1;
	}
	else if (secret[0] == 0x02)
	{
		ecname = 2;
	}
	else if (secret[0] == 0x03)
	{
		ecname = 3;
	}
	get_curve_parameters(ecname, c);
	//导入私钥
	mpz_init_set_str(k, key, 16);
	//导入密文
	cipherdata_length_byte = cipherdata_length_byte - 1;
	blocklength_byte = 4 * c->length / 8;						//块大小
	blocknum = (int)(cipherdata_length_byte / blocklength_byte);		//分块数
	cipherdata = malloc(sizeof(unsigned char *)*blocknum);
	for (i = 0; i < blocknum; i++)
	{
		cipherdata[i] = secret + 1 + blocklength_byte * i;
	}
	//为明文分配内存
	plain_blocklength_byte = 2 * (c->length / 8 - 1);
	plain = malloc(sizeof(unsigned char)*blocknum*plain_blocklength_byte);
	plaindata = malloc(sizeof(unsigned char *)*blocknum);
	for (i = 0; i < blocknum; i++)
	{
		plaindata[i] = plain + plain_blocklength_byte*i;
	}
	//多线程解密
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
			t[i]->k = k;
			t[i]->plaindata = malloc(sizeof(unsigned char *)*bnft);
			t[i]->cipherdata = malloc(sizeof(unsigned char *)*bnft);
			for (j = 0; j < bnft; j++)
			{
				t[i]->plaindata[j] = plaindata[i*bnft + j];
				t[i]->cipherdata[j] = cipherdata[i*bnft + j];
			}
			threadsignal[i] = _beginthread(decrypt_thread, 0, t[i]);
		}
		if (lastbnft != 0)
		{
			t[threadnum] = malloc(sizeof(threadarg));
			t[threadnum]->threadnum = threadnum;
			t[threadnum]->blocklength_byte = blocklength_byte;
			t[threadnum]->blocknum = lastbnft;
			t[threadnum]->ecname = ecname;
			t[threadnum]->c = c;
			t[threadnum]->k = k;
			t[threadnum]->plaindata = malloc(sizeof(unsigned char *)*lastbnft);
			t[threadnum]->cipherdata = malloc(sizeof(unsigned char *)*lastbnft);
			for (j = 0; j < lastbnft; j++)
			{
				t[threadnum]->plaindata[j] = plaindata[threadnum*bnft + j];
				t[threadnum]->cipherdata[j] = cipherdata[threadnum*bnft + j];
			}
			threadsignal[threadnum] = _beginthread(decrypt_thread, 0, t[threadnum]);
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
		t[0]->k = k;
		t[0]->plaindata = malloc(sizeof(unsigned char *)*blocknum);
		t[0]->plaindata[0] = plaindata[0];
		t[0]->cipherdata = malloc(sizeof(unsigned char *)*blocknum);
		t[0]->cipherdata[0] = cipherdata[0];
		for (j = 0; j < blocknum; j++)
		{
			t[0]->plaindata[j] = plaindata[j];
			t[0]->cipherdata[j] = cipherdata[j];
		}
		decrypt_thread(t[0]);
		free(t[0]->plaindata);
		free(t[0]->cipherdata);
		free(t[0]);
	}
	memcpy(plaindata_length_byte, plain, 8);
	memmove(plain, plain + 8, sizeof(unsigned char)*(blocknum*plain_blocklength_byte - 8));

	//释放内存
	mpz_clear(k);
	group_clears(c);
	free(plaindata);
	free(cipherdata);
	free(t);
	return plain;
}
