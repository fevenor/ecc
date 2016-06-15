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
	double t1, t2;
	mpz_t m, t;
	mpz_init_set(m, k);
	mpz_init(t);
	t1 = pow(2, w);
	t2 = pow(2, w - 1);
	for (i = 0; mpz_cmp_d(m, 0); i++)			//当k不为0时
	{
		if (mpz_odd_p(m))						//如果k是奇数
		{
			mpz_mod_ui(t, m, t1);					//t=k mod 2^w
			if (mpz_cmp_d(t, t2) == -1)			//如果t<2^(w-1)
			{
				naf_k->n[i] = 1;				//ki为正数
				naf_k->v[i] = mpz_get_ui(t);	//ki=t
												//naf_k->v[i] = t->_mp_d;			//ki=t
				mpz_sub(m, m, t);				//k=k-ki
			}
			else
			{
				naf_k->n[i] = 0;				//ki为负数
				mpz_ui_sub(t, t1, t);				//|ki|=2^w-t
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
	mpz_clears(m, t, NULL);
	return naf_k;
}

void scalar_mul_w(int w, mpz_t k, af_p *p, af_p *q, group *c)
{
	int i, pre_p_n, k_length;
	double pre_p_max;
	ja_p *(*pre_p);
	ja_p *temp = ja_p_inits();
	ja_p *jap = ja_p_inits();
	ja_p *jaq = ja_p_inits();
	af2ja(p, jap);

	//预计算iP的值
	pre_p_max = pow(2, w - 1) - 1;						//确认最大点的值
	pre_p_n = (pre_p_max - 1) / 2;						//确认点的个数
	pre_p = malloc(sizeof(ja_p*)*(pre_p_n + 1));			//预分配内存
	for (i = 0; i <= pre_p_n; i++)
	{
		pre_p[i] = ja_p_inits();
	}
	point_set(pre_p[0], jap);								//令p1=p
	ja_p *dp = ja_p_inits();
	point_double(c->p, jap, dp);
	for (i = 1; i <= pre_p_n; i++)
	{
		point_add(c->p, pre_p[i - 1], dp, pre_p[i]);
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
		point_double(c->p, jaq, jaq);				//Q=2Q
		if (naf_k->v[i] != 0)
		{

			if (naf_k->n[i] == 1)
			{
				point_add(c->p, jaq, pre_p[(naf_k->v[i] - 1) / 2], jaq);	//Q=Q+Pki
			}
			else
			{
				point_neg(temp, pre_p[(naf_k->v[i] - 1) / 2]);
				point_add(c->p, jaq, temp, jaq);							//Q=Q-Pki
			}
		}
		else
		{
			if (i % 2 == 0)
				point_add(c->p, temp, pre_p[0], temp);							//虚操作
		}
	}

	ja2af(c->p, jaq, q);

	//释放内存
	ja_p_clears(jap);
	ja_p_clears(jaq);
	ja_p_clears(dp);
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

void scalar_mul_c(mpz_t k, af_p *q, enum curve_name ecname)
{
	int i, n0, n1;
	ja_p *jaq = ja_p_inits();
	ja_p *temp = ja_p_inits();

	//获得预计算的值及椭圆参数
	ja_p *pre_p[2][16];
	group *c = group_inits();
	for (i = 0; i < 16; i++)
	{
		pre_p[0][i] = ja_p_inits();
		pre_p[1][i] = ja_p_inits();
	}
	get_pre_cal_value(ecname, pre_p);
	get_curve_parameters(ecname, c);

	//计算长度，初始化矩阵
	int k_length = mpz_sizeinbase(k, 2);
	int k_c_length;
	int *k_c[4], *k_int;
	char *k_char;
	k_c_length = c->length / 4;
	k_char = malloc(sizeof(char)*(c->length + 1));	//char格式的k
	k_int = malloc(sizeof(int)*c->length);		//int数组格式的k
	mpz_get_str(k_char, 2, k);
	if (k_length > c->length)					//当k大于密钥长度
	{
		for (i = 0; i < c->length; i++)
		{
			k_int[i] = k_char[k_length - 1 - i] - '0';
		}
	}
	else if (k_length < c->length)				//当k小于密钥长度
	{
		for (i = 0; i < k_length; i++)
		{
			k_int[i] = k_char[k_length - 1 - i] - '0';
		}
		for (i = k_length; i < c->length; i++)
		{
			k_int[i] = 0;
		}
	}
	else										//当k等于密钥长度
	{
		for (i = 0; i < c->length; i++)
		{
			k_int[i] = k_char[k_length - 1 - i] - '0';
		}
	}
	for (i = 0; i < 4; i++)						//将k分为4行
	{
		k_c[i] = k_int + (k_c_length*i);
	}

	//主循环部分
	for (i = k_c_length / 2 - 1; i >= 0; i--)
	{
		point_double(c->p, jaq, jaq);
		n0 = (((k_c[3][i] * 2 + k_c[2][i]) * 2) + k_c[1][i]) * 2 + k_c[0][i];
		n1 = (((k_c[3][i + k_c_length / 2] * 2 + k_c[2][i + k_c_length / 2]) * 2) + k_c[1][i + k_c_length / 2]) * 2 + k_c[0][i + k_c_length / 2];
		if (n0 != 0)
		{
			point_add(c->p, jaq, pre_p[0][n0], jaq);
		}
		else
		{
			if (i % 2 == 0)
				point_add(c->p, temp, pre_p[0][1], temp);							//虚操作
		}
		if (n1 != 0)
		{
			point_add(c->p, jaq, pre_p[1][n1], jaq);
		}
		else
		{
			if (i % 2 == 0)
				point_add(c->p, temp, pre_p[1][1], temp);							//虚操作
		}
	}

	ja2af(c->p, jaq, q);

	//释放内存
	group_clears(c);
	ja_p_clears(jaq);
	ja_p_clears(temp);
	for (i = 0; i < 16; i++)
	{
		ja_p_clears(pre_p[0][i]);
		ja_p_clears(pre_p[1][i]);
	}
	free(k_char);
	free(k_int);
}
