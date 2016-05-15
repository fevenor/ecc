#include "basefunc.h"

key* get_key(enum curve_name ecname)
{
	key *newkey;
	mpz_t key;
	af_p *pub_p = af_p_inits();
	group *c = group_inits();
	mpz_init(key);
	get_curve_parameters(ecname, c);
	get_rand(c->length, key);
	mpz_mod(key, key, c->n);
	newkey = malloc(sizeof(key) + sizeof(char) *((c->length / 8 + 1) * 3 + 10));
	newkey->curve = malloc(sizeof(char) * 10);
	newkey->private = malloc(sizeof(char)*(c->length / 8 + 1));
	newkey->public_x = malloc(sizeof(char)*(c->length / 8 + 1));
	newkey->public_y = malloc(sizeof(char)*(c->length / 8 + 1));
	scalar_mul_c(key, pub_p, ecname);
	mpz_get_str(newkey->private, 16, key);
	mpz_get_str(newkey->public_x, 16, pub_p->x);
	mpz_get_str(newkey->public_y, 16, pub_p->y);
	af_p_clears(pub_p);
	if (ecname == 0)
	{
		newkey->curve = "secp160r2";
	}
	else if (ecname == 1)
	{
		newkey->curve = "secp192r1";
	}
	else if (ecname == 2)
	{
		newkey->curve = "secp224r1";
	}
	else if (ecname == 3)
	{
		newkey->curve = "secp256r1";
	}
	return newkey;
}