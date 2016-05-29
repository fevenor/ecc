#include "basefunc.h"

//������
static HANDLE mutex = NULL;

//�߳��ź�
static HANDLE *threadsignal;

//���߲���
static group *c;

//�̲߳���
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

//�����߳�
void decrypt_thread(threadarg *t)
{
	int i, j, length_diff;
	mpz_t m;
	mpz_init(m);
	unsigned char *temp = malloc(sizeof(unsigned char)*(c->length / 8 - 1));
	for (i = 0; i < t->blocknum; i++)
	{
		//����������������ת��
		mpz_import(m, t->blocklength_byte, 1, sizeof(unsigned char), 0, 0, t->cipherdata[i]);
		//��P����
		mpz_sub(m, m, t->p->y);
		mpz_mod(m, m, c->p);
		mpz_mul(m, m, t->inverse_x);
		mpz_mod(m, m, c->p);
		//��������������ת��
		length_diff = (c->length / 4 - 2 - (int)mpz_sizeinbase(m, 16)) / 2;
		for (j = 0; j < length_diff; j++)
		{
			temp[j] = 0x00;
		}
		mpz_export(temp + length_diff, NULL, 1, sizeof(unsigned char), 0, 0, m);
		//��ȡ��ʵ��Ϣ
		int salt_location = (int)temp[0] % (c->length / 16);
		memcpy(t->plaindata[i], temp + 1, salt_location);
		memcpy(t->plaindata[i] + salt_location, temp + 1 + salt_location + c->length / 16 - 2, c->length / 16 - salt_location);
	}

	//�ͷ��ڴ�
	mpz_clear(m);
	free(temp);
}


//����ģ��
unsigned char* decrypt(char *key, unsigned char *secret, unsigned long long cipherdata_length_byte, unsigned long long *plaindata_length_byte)
{
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
	SYSTEM_INFO siSysInfo;
	threadarg *(*t);
	//������߲���
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
	//����˽Կ
	mpz_init_set_str(k, key, 16);
	//����E��
	mpz_import(e->x, c->length / 8, 1, sizeof(unsigned char), 0, 0, secret + 1);
	mpz_import(e->y, c->length / 8, 1, sizeof(unsigned char), 0, 0, secret + 1 + c->length / 8);
	//����P=nB(kG)=k(nBG)=kPB
	scalar_mul_w(w, k, e, p, c);
	mpz_init(inverse_x);
	mpz_invert(inverse_x, p->x, c->p);
	//��������
	cipherdata_length_byte = cipherdata_length_byte - 1 - c->length / 4;
	blocklength_byte = c->length / 8;						//���С
	blocknum = (int)(cipherdata_length_byte / blocklength_byte);		//�ֿ���
	cipherdata = malloc(sizeof(unsigned char *)*blocknum);
	for (i = 0; i < blocknum; i++)
	{
		cipherdata[i] = secret + 1 + c->length / 4 + blocklength_byte * i;
	}
	//Ϊ���ķ����ڴ�
	plain_blocklength_byte = c->length / 16;
	plain = malloc(sizeof(unsigned char)*blocknum*plain_blocklength_byte);
	plaindata = malloc(sizeof(unsigned char *)*blocknum);
	for (i = 0; i < blocknum; i++)
	{
		plaindata[i] = plain + plain_blocklength_byte*i;
	}
	//���߳̽��ܴ���
	GetSystemInfo(&siSysInfo);		//���ϵͳ��Ϣ
	threadnum = siSysInfo.dwNumberOfProcessors;		//�����߼�����������ȷ����Ҫ�߳���
	if (blocknum >= threadnum)						//�����Ҫ����Ŀ����㹻�࣬���̴߳���
	{
		bnft = blocknum / threadnum;			//ÿ���̴߳������
		lastbnft = blocknum % threadnum;		//���ܶ���Ŀ���
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
			mpz_init_set(t[i]->inverse_x, inverse_x);
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
			t[threadnum]->p = p;
			mpz_init_set(t[threadnum]->inverse_x, inverse_x);
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
				mpz_clear(t[i]->inverse_x);
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
	memmove(plain, plain + 8, sizeof(unsigned char)*(blocknum*plain_blocklength_byte - 8));

	//�ͷ��ڴ�
	mpz_clear(k);
	mpz_clear(inverse_x);
	group_clears(c);
	af_p_clears(p);
	af_p_clears(e);
	free(plaindata);
	free(cipherdata);
	return plain;
}
