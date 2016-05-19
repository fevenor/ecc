#include "basefunc.h"

int get_key(char *curve, char *privatekey, char *public_x, char *public_y)
{
	enum curve_name ecname;
	mpz_t key;
	af_p *pub_p = af_p_inits();
	group *c = group_inits();
	mpz_init(key);
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
	else
	{
		return 1;
	}
	get_curve_parameters(ecname, c);
	get_rand(c->length, key);
	mpz_mod(key, key, c->n);
	scalar_mul_c(key, pub_p, ecname);
	mpz_get_str(privatekey, 16, key);
	mpz_get_str(public_x, 16, pub_p->x);
	mpz_get_str(public_y, 16, pub_p->y);
	//释放内存
	mpz_clear(key);
	//af_p_clears(pub_p);
	//group_clears(c);
	return 0;
}
