#include "libcal.h"


af_p* af_p_inits()
{
	af_p *p = malloc(sizeof(*p) + sizeof(mpz_t) * 2);
	mpz_inits(p->x, p->y, NULL);
	return p;
}
af_p* af_p_inits_set_d(double x, double y)
{
	af_p *p = malloc(sizeof(*p) + sizeof(mpz_t) * 2);
	mpz_init_set_d(p->x, x);
	mpz_init_set_d(p->y, y);
	return p;
}
ja_p* ja_p_inits()
{
	ja_p *p = malloc(sizeof(*p) + sizeof(mpz_t) * 4);
	mpz_inits(p->x, p->y, p->z, p->zz, NULL);
	return p;
}



void af_p_clears(af_p *p)
{
	mpz_clears(p->x, p->y, NULL);
}

void ja_p_clears(ja_p *p)
{
	mpz_clears(p->x, p->y, p->z, p->zz, NULL);
}

int af2ja(af_p *af, ja_p *ja)
{
	mpz_set(ja->x, af->x);
	mpz_set(ja->y, af->y);
	mpz_set_d(ja->z, 1);
	mpz_set_d(ja->zz, 1);
	return 0;
}

int ja2af(mpz_t p, ja_p *ja, af_p *af)
{
	mpz_t t;
	mpz_init(t);
	//求x
	mpz_invert(t, ja->zz, p);
	mpz_mul(af->x, ja->x, t);
	mpz_mod(af->x, af->x, p);
	//求y
	mpz_mul(t, ja->z, ja->zz);
	mpz_mod(t, t, p);
	mpz_invert(t, t, p);
	mpz_mul(af->y, ja->y, t);
	mpz_mod(af->y, af->y, p);
	mpz_clear(t);
	return 0;
}

int point_add(mpz_t p, ja_p *p1, ja_p *p2, ja_p *p3)
{

	if ((mpz_cmp(p1->x, p2->x) == 0) & (mpz_cmp(p1->y, p2->y) == 0) & (mpz_cmp(p1->z, p2->z) == 0))	//如果输入两数相等，进行倍点运算
	{
		point_double(p, p1, p3);
		return 0;
	}
	else if ((mpz_cmp_d(p1->x, 0) == 0) & (mpz_cmp_d(p1->y, 0) == 0))	//当其中一点为0点，直接输出另一点
	{
		mpz_set(p3->x, p2->x);
		mpz_set(p3->y, p2->y);
		mpz_set(p3->z, p2->z);
		mpz_set(p3->zz, p2->zz);
		return 0;
	}
	else if ((mpz_cmp_d(p2->x, 0) == 0) & (mpz_cmp_d(p2->y, 0) == 0))
	{
		mpz_set(p3->x, p1->x);
		mpz_set(p3->y, p1->y);
		mpz_set(p3->z, p1->z);
		mpz_set(p3->zz, p1->zz);
		return 0;
	}
	mpz_t t, a1, a2, a3, b1, b2, c1, c2, d1, d2;
	mpz_inits(t, a1, a2, a3, b1, b2, c1, c2, d1, d2, NULL);

	//B1=X1*Z2^2
	mpz_mul(b1, p1->x, p2->zz);
	//B2=X2*Z1^2
	mpz_mul(b2, p2->x, p1->zz);
	//C1=Y1*(Z2*Z2^2)
	mpz_mul(c1, p2->z, p2->zz);
	mpz_mul(c1, p1->y, c1);
	//C2=Y2*(Z1*Z1^2)
	mpz_mul(c2, p1->z, p1->zz);
	mpz_mul(c2, p2->y, c2);
	//A1=2(C2-C1)
	mpz_sub(a1, c2, c1);
	mpz_add(a1, a1, a1);
	//A2=B2-B1
	mpz_sub(a2, b2, b1);
	//A3=(2A2)^2
	mpz_add(a3, a2, a2);
	mpz_mul(a3, a3, a3);
	//D1=A2*A3
	mpz_mul(d1, a2, a3);
	//D2=B1*A3
	mpz_mul(d2, b1, a3);
	//X3=(A1)^2-D1-2D2
	mpz_mul(p3->x, a1, a1);
	mpz_sub(p3->x, p3->x, d1);
	mpz_sub(p3->x, p3->x, d2);
	mpz_sub(p3->x, p3->x, d2);
	mpz_mod(p3->x, p3->x, p);
	//Y3=A1*(D2-X3)-2C1*D1
	mpz_sub(p3->y, d2, p3->x);
	mpz_mul(p3->y, a1, p3->y);
	mpz_mul(t, c1, d1);
	mpz_sub(p3->y, p3->y, t);
	mpz_sub(p3->y, p3->y, t);
	mpz_mod(p3->y, p3->y, p);
	//Z3=((Z1+Z2)^2-Z1^2-Z2^2)*A2
	mpz_add(p3->z, p1->z, p2->z);
	mpz_mul(p3->z, p3->z, p3->z);
	mpz_sub(p3->z, p3->z, p1->zz);
	mpz_sub(p3->z, p3->z, p2->zz);
	mpz_mul(p3->z, p3->z, a2);
	mpz_mod(p3->z, p3->z, p);
	//Z3^2=(Z3 )^2
	mpz_mul(p3->zz, p3->z, p3->z);
	mpz_clears(t, a1, a2, a3, b1, b2, c1, c2, d1, d2, NULL);
	return 0;
}

int point_double(mpz_t p, ja_p *p1, ja_p *p3)
{
	if ((mpz_cmp_d(p1->y, 0) == 0) | (mpz_cmp_d(p1->z, 0) == 0))	//如果输入的是0点，输出0点
	{
		mpz_set_d(p3->x, 0);
		mpz_set_d(p3->y, 0);
		mpz_set_d(p3->z, 1);
		mpz_set_d(p3->zz, 1);
		return 0;
	}
	mpz_t t1, t2, x1x1, a1, a2, a3, b1, z1_4;
	mpz_inits(t1, t2, x1x1, a1, a2, a3, b1, z1_4, NULL);

	//将a4设为1，用于调试
	mpz_t a;
	mpz_init_set_d(a, 1);

	//B1=Y1^2
	mpz_mul(b1, p1->y, p1->y);
	if (mpz_cmp_d(a, -3) != 0)
	{
		//A1=3X1^2+a(Z1^2)^2
		mpz_mul(x1x1, p1->x, p1->x);
		mpz_mul(z1_4, p1->zz, p1->zz);
		mpz_mul(a1, a, z1_4);
		mpz_add(a1, a1, x1x1);
		mpz_add(a1, a1, x1x1);
		mpz_add(a1, a1, x1x1);
	}
	else
	{
		//当a=-3时,可以进一步减少计算
		//A1=3(X1+Z1^2)(X1-Z1^2)
		mpz_add(t1, p1->x, p1->zz);
		mpz_sub(t2, p1->x, p1->zz);
		mpz_mul(t1, t1, t2);
		mpz_add(a1, t1, t1);
		mpz_add(a1, a1, t1);
	}
	//A2=4X1*B1
	mpz_mul(a2, p1->x, b1);
	mpz_add(a2, a2, a2);
	mpz_add(a2, a2, a2);
	//A3=8B1^2
	mpz_mul(a3, b1, b1);
	mpz_add(a3, a3, a3);
	mpz_add(a3, a3, a3);
	mpz_add(a3, a3, a3);
	//X3=A1^2-2A2
	mpz_mul(p3->x, a1, a1);
	mpz_sub(p3->x, p3->x, a2);
	mpz_sub(p3->x, p3->x, a2);
	mpz_mod(p3->x, p3->x, p);
	//Z3=(Y1+Z1)^2-B1-Z1^2
	mpz_add(p3->z, p1->y, p1->z);
	mpz_mul(p3->z, p3->z, p3->z);
	mpz_sub(p3->z, p3->z, b1);
	mpz_sub(p3->z, p3->z, p1->zz);
	mpz_mod(p3->z, p3->z, p);
	//Y3=A1*(A2-X3)-A3
	mpz_sub(p3->y, a2, p3->x);
	mpz_mul(p3->y, a1, p3->y);
	mpz_sub(p3->y, p3->y, a3);
	mpz_mod(p3->y, p3->y, p);
	//Z3Z3=Z3^2
	mpz_mul(p3->zz, p3->z, p3->z);
	mpz_clears(a, t1, t2, x1x1, a1, a2, a3, b1, z1_4, NULL);
	return 0;
}