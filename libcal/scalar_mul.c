#include "libcal.h"
#include "math.h"

//NAFw数
typedef struct nafw_number
{
	//l从0开始计数
	int l;
	int *n;
	int *v;
}naf;

naf* bin2nafw(int w, mpz_t k, naf *naf_k)
{
	int i;
	double t2;
	mpz_t m, t, t1;
	mpz_init_set(m, k);
	mpz_init(t);
	mpz_init_set_d(t1, pow(2, w));
	t2 = pow(2, w - 1);
	for (i = 0; mpz_cmp_d(m, 0); i++)			//当k不为0时
	{
		if (mpz_odd_p(m))						//如果k是奇数
		{
			mpz_mod(t, m, t1);					//t=k mod 2^w
			if (mpz_cmp_d(t, t2) == -1)			//如果t<2^(w-1)
			{
				naf_k->n[i] = 1;				//ki为正数
				naf_k->v[i] = mpz_get_ui(t);	//ki=t
				mpz_sub(m, m, t);				//k=k-ki
			}
			else
			{
				naf_k->n[i] = 0;				//ki为负数
				mpz_sub(t, t1, t);				//|ki|=2^w-t
				naf_k->v[i] = mpz_get_ui(t);
				mpz_add(m, m, t);				//k=k+(-ki)
			}
		}
		else
		{
			naf_k->n[i] = 1;					//ki=0
			naf_k->v[i] = 0;
		}
		mpz_divexact_ui(m, m, 2);				//k=k/2
	}
	naf_k->l = i - 1;
	return naf_k;
}

void scalar_mul_w(int w, mpz_t k, ja_p *p, ja_p *q, mpz_t prime)
{
	int i, pre_p_n, k_length;
	double pre_p_max;
	ja_p *(*pre_p);
	ja_p *temp = ja_p_inits();

	//预计算iP的值
	pre_p_max = pow(2, w - 1) - 1;						//确认最大点的值
	pre_p_n = (pre_p_max - 1) / 2;						//确认点的个数
	pre_p = malloc(sizeof(ja_p)*(pre_p_n + 1));			//预分配内存
	for (i = 0; i <= pre_p_n; i++)
	{
		pre_p[i] = ja_p_inits();
	}
	point_set(pre_p[0], p);								//令p1=p
	ja_p *c = ja_p_inits();
	point_double(prime, p, c);
	for (i = 1; i <= pre_p_n; i++)
	{
		point_add(prime, pre_p[i - 1], c, pre_p[i]);
	}

	//对k进行NAFw编码转换
	k_length = mpz_sizeinbase(k, 2) + 1;
	naf *naf_k = malloc(sizeof(naf));
	naf_k->n = malloc(sizeof(int) * k_length);
	naf_k->v = malloc(sizeof(int) * k_length);
	bin2nafw(w, k, naf_k);

	//主循环部分
	for (i = naf_k->l; i >= 0; i--)
	{
		point_double(prime, q, q);				//Q=2Q
		if (naf_k->v[i] != 0)
		{

			if (naf_k->n[i] == 1)
			{
				point_add(prime, q, pre_p[(naf_k->v[i] - 1) / 2], q);	//Q=Q+Pki
			}
			else
			{
				point_neg(temp, pre_p[(naf_k->v[i] - 1) / 2]);
				point_add(prime, q, temp, q);							//Q=Q-Pki
			}
		}
	}


	//释放iP占用的内存
	for (i = 0; i <= pre_p_n; i++)
	{
		ja_p_clears(pre_p[i]);
	}
	free(pre_p);
	ja_p_clears(temp);
	//释放NAFw编码占用的内存
	free(naf_k->v);
	free(naf_k->n);
	free(naf_k);
}