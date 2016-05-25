#include "basefunc.h"

//����ģ��
unsigned char* decrypt(char *key, unsigned char *secret, unsigned long long cipherdata_length_byte, unsigned long long *plaindata_length_byte)
{
	int w = 8;
	int i, j, blocknum, blocklength_byte, plain_blocklength_byte, length_diff;
	mpz_t k, inverse_x;
	group *c = group_inits();
	unsigned char *(*plaindata);
	unsigned char *(*cipherdata);
	unsigned char *plain;
	enum curve_name ecname;
	af_p *p = af_p_inits();
	af_p *e = af_p_inits();
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
	//����
	mpz_t m;
	mpz_init(m);
	for (i = 0; i < blocknum; i++)
	{
		//����������������ת��
		mpz_import(m, blocklength_byte, 1, sizeof(unsigned char), 0, 0, cipherdata[i]);
		//��P����
		mpz_sub(m, m, p->y);
		mpz_mod(m, m, c->p);
		mpz_mul(m, m, inverse_x);
		mpz_mod(m, m, c->p);
		mpz_tdiv_q_2exp(m, m, c->length / 2 - 8);
		//��������������ת��
		length_diff = (c->length / 8 - (int)mpz_sizeinbase(m, 16)) / 2;
		for (j = 0; j < length_diff; j++)
		{
			plaindata[i][j] = 0x00;
		}
		mpz_export(plaindata[i] + length_diff, NULL, 1, sizeof(unsigned char), 0, 0, m);
	}
	memcpy(plaindata_length_byte, plain, 8);
	memmove(plain, plain + 8, sizeof(unsigned char)*(blocknum*plain_blocklength_byte - 8));

	//�ͷ��ڴ�
	mpz_clear(m);
	mpz_clear(k);
	mpz_clear(inverse_x);
	group_clears(c);
	af_p_clears(p);
	af_p_clears(e);
	free(plaindata);
	free(cipherdata);
	return plain;
}
