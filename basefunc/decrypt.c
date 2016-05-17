#include "basefunc.h"

//������
static HANDLE mutex = NULL;

//�߳��ź�
static HANDLE *threadsignal;

//�̲߳���
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

//�����߳�
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
		//������ת��Ϊ��
		mpz_import(e->x, t->blocklength_byte / 4, 1, sizeof(unsigned char), 0, 0, t->cipherdata[i]);
		mpz_import(e->y, t->blocklength_byte / 4, 1, sizeof(unsigned char), 0, 0, t->cipherdata[i] + (t->blocklength_byte / 4));
		mpz_import(c->x, t->blocklength_byte / 4, 1, sizeof(unsigned char), 0, 0, t->cipherdata[i] + 2 * (t->blocklength_byte / 4));
		mpz_import(c->y, t->blocklength_byte / 4, 1, sizeof(unsigned char), 0, 0, t->cipherdata[i] + 3 * (t->blocklength_byte / 4));
		//����P=nB(kG)=k(nBG)=kPB
		scalar_mul_w(w, t->k, e, p, t->c);
		//��P����
		mpz_sub(m->x, c->x, p->y);
		mpz_invert(temp, p->x, t->c->p);
		mpz_mul(m->x, m->x, temp);
		mpz_mod(m->x, m->x, t->c->p);
		mpz_sub(m->y, c->y, p->x);
		mpz_invert(temp, p->y, t->c->p);
		mpz_mul(m->y, m->y, temp);
		mpz_mod(m->y, m->y, t->c->p);
		//��������������ת��
		partlength = t->c->length / 8 - 1;
		//M��xֵ
		length_diff = (t->c->length / 4 - 2 - (int)mpz_sizeinbase(m->x, 16)) / 2;
		for (j = 0; j < length_diff; j++)
		{
			t->plaindata[i][j] = 0x00;
		}
		mpz_export(t->plaindata[i] + length_diff, NULL, 1, sizeof(unsigned char), 0, 0, m->x);
		//M��yֵ
		length_diff = (t->c->length / 4 - 2 - (int)mpz_sizeinbase(m->y, 16)) / 2;
		for (j = 0; j < length_diff; j++)
		{
			t->plaindata[i][j + partlength] = 0x00;
		}
		mpz_export(t->plaindata[i] + partlength + length_diff, NULL, 1, sizeof(unsigned char), 0, 0, m->y);
	}

	//�ͷ��ڴ�
	af_p_clears(m);
	af_p_clears(p);
	af_p_clears(e);
	af_p_clears(c);
	mpz_clear(temp);
}

//����ģ��
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
	//��������
	cipherdata_length_byte = cipherdata_length_byte - 1;
	blocklength_byte = 4 * c->length / 8;						//���С
	blocknum = (int)(cipherdata_length_byte / blocklength_byte);		//�ֿ���
	cipherdata = malloc(sizeof(unsigned char *)*blocknum);
	for (i = 0; i < blocknum; i++)
	{
		cipherdata[i] = secret + 1 + blocklength_byte * i;
	}
	//Ϊ���ķ����ڴ�
	plain_blocklength_byte = 2 * (c->length / 8 - 1);
	plain = malloc(sizeof(unsigned char)*blocknum*plain_blocklength_byte);
	plaindata = malloc(sizeof(unsigned char *)*blocknum);
	for (i = 0; i < blocknum; i++)
	{
		plaindata[i] = plain + plain_blocklength_byte*i;
	}
	//���߳̽���
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

	//�ͷ��ڴ�
	mpz_clear(k);
	group_clears(c);
	free(plaindata);
	free(cipherdata);
	free(t);
	return plain;
}
