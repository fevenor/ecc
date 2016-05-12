#include "libcal.h"

int get_curve_parameters(enum curve_name ecname, group *c)
{

	char curve_parameters[][6][65] =
	{
		//secp160r2
		{
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFAC73",		//Prime			0
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFAC70",		//A				1
			"B4E134D3FB59EB8BAB57274904664D5AF50388BA",		//B				2
			"52DCB034293A117E1F4FF11B30F7199D3144CE6D",		//Generator_x	3
			"FEAFFEF2E331F296E071FA0DF9982CFEA7D43F2E",		//Generator_y	4
			"00000000000000000000351EE786A818F3A1A16B"		//Order			5
		},
		//secp192r1
		{
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFF",
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFC",
			"64210519E59C80E70FA7E9AB72243049FEB8DEECC146B9B1",
			"188DA80EB03090F67CBF20EB43A18800F4FF0AFD82FF1012",
			"07192B95FFC8DA78631011ED6B24CDD573F977A11E794811",
			"FFFFFFFFFFFFFFFFFFFFFFFF99DEF836146BC9B1B4D22831"
		},
		//secp224r1
		{
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000001",
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFE",
			"B4050A850C04B3ABF54132565044B0B7D7BFD8BA270B39432355FFB4",
			"B70E0CBD6BB4BF7F321390B94A03C1D356C21122343280D6115C1D21",
			"BD376388B5F723FB4C22DFE6CD4375A05A07476444D5819985007E34",
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFF16A2E0B8F03E13DD29455C5C2A3D"
		},
		//secp256r1
		{
			"FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF",
			"FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC",
			"5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B",
			"6B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296",
			"4FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5",
			"FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551"
		}
	};

	if (ecname > 3)
	{
		return -1;
	}

	mpz_set_str(c->p, curve_parameters[ecname][0], 16);
	mpz_set_str(c->a, curve_parameters[ecname][1], 16);
	mpz_set_str(c->b, curve_parameters[ecname][2], 16);
	mpz_set_str(c->g->x, curve_parameters[ecname][3], 16);
	mpz_set_str(c->g->y, curve_parameters[ecname][4], 16);
	mpz_set_str(c->n, curve_parameters[ecname][5], 16);
	mpz_set_d(c->g->z, 1);
	mpz_set_d(c->g->zz, 1);
	c->length = mpz_sizeinbase(c->p, 2);
	return 0;
}

void pre_cal(enum curve_name ecname, ja_p *pre_p[])
{
	/*
	DEC|BIN|Point
	--+----+--------------------------------------------
	0 |0000|0G
	1 |0001|1G
	2 |0010|(2^k_c_length)G
	3 |0011|(2^k_c_length+1)G
	4 |0100|(2^2k_c_length)G
	5 |0101|(2^2k_c_length+1)G
	6 |0110|(2^2k_c_length+2 ^k_c_length)G
	7 |0111|(2^2k_c_length+2 ^k_c_length + 1)G
	8 |1000|(2^3k_c_length)G
	9 |1001|(2^3k_c_length+1)G
	10|1010|(2^3k_c_length+2^56)G
	11|1011|(2^3k_c_length+2^56+1)G
	12|1100|(2^3k_c_length+2^112)G
	13|1101|(2^3k_c_length+2^2k_c_length+1)G
	14|1110|(2^3k_c_length+2^2k_c_length+2^k_c_length)G
	15|1111|(2^3k_c_length+2^2k_c_length+2^k_c_length+1)G
	*/

	int i, k_c_length;
	group *c = group_inits();
	get_curve_parameters(ecname, c);
	k_c_length = c->length / 4;
	af_p *temp;
	temp = af_p_inits();
	//0000
	mpz_set_d(pre_p[0]->z, 1);
	mpz_set_d(pre_p[0]->zz, 1);
	//0001
	point_set(pre_p[1], c->g);
	//0010
	point_set(pre_p[2], pre_p[1]);
	for (i = 0; i < k_c_length; i++)
	{
		point_double(c->p, pre_p[2], pre_p[2]);
	}
	//0011
	point_add(c->p, pre_p[1], pre_p[2], pre_p[3]);
	//0100
	point_set(pre_p[4], pre_p[2]);
	for (i = 0; i < k_c_length; i++)
	{
		point_double(c->p, pre_p[4], pre_p[4]);
	}
	//0101
	point_add(c->p, pre_p[1], pre_p[4], pre_p[5]);
	//0110
	point_add(c->p, pre_p[2], pre_p[4], pre_p[6]);
	//0111
	point_add(c->p, pre_p[1], pre_p[6], pre_p[7]);
	//1000
	point_set(pre_p[8], pre_p[4]);
	for (i = 0; i < k_c_length; i++)
	{
		point_double(c->p, pre_p[8], pre_p[8]);
	}
	//1001
	point_add(c->p, pre_p[1], pre_p[8], pre_p[9]);
	//1010
	point_add(c->p, pre_p[2], pre_p[8], pre_p[10]);
	//1011
	point_add(c->p, pre_p[1], pre_p[10], pre_p[11]);
	//1100
	point_add(c->p, pre_p[4], pre_p[8], pre_p[12]);
	//1101
	point_add(c->p, pre_p[1], pre_p[12], pre_p[13]);
	//1110
	point_add(c->p, pre_p[2], pre_p[12], pre_p[14]);
	//1111
	point_add(c->p, pre_p[1], pre_p[14], pre_p[15]);
	for (i = 0; i < 16; i++)
	{
		ja2af(c->p, pre_p[i], temp);
		af2ja(temp, pre_p[i]);
	}

}

int get_pre_cal_value(enum curve_name ecname, ja_p *pre_p[])
{
	char pre_cal_values[][16][2][65] =
	{
		//secp160r2
		{
			//pre_p[1]
			{
				"0",
				"0"
			},
		//pre_p[1]
			{
				"52dcb034293a117e1f4ff11b30f7199d3144ce6d",
				"feaffef2e331f296e071fa0df9982cfea7d43f2e"
			},
		//pre_p[2]
			{
				"3814a549594c44ce6caea79745742ea589abdd96",
				"5b0a002d8e794b12cfb2ff8a2eaf75ac3ffe95d4"
			},
		//pre_p[3]
			{
				"c74b5cdd6c896d1399fc66513e49592b23cfd102",
				"1f6c85e2b2275b985df7d96582405fd505aa04f5"
			},
		//pre_p[4]
			{
				"af6092be029e4d9b06a1961f2dd8da5d9fdbe7cc",
				"3e9221cddcd3fb29952f7d3556d0ee4731a7c90d"
			},
		//pre_p[5]
			{
				"2a9f47a18c7f44dadadbb9346222cbb159aa3451",
				"b78c590839ffbb73511c79a14833968a23687833"
			},
		//pre_p[6]
			{
				"8d4a4a4c748b0fbc8f97d00419fee8fe876283a2",
				"d94e3c0416fe54e443bc096d12849a187b4dfb9d"
			},
		//pre_p[7]
			{
				"f2b642273aaca989f9028434b489efa1dd6eb53a",
				"a4c3839b5a6d6d5f314068728a059262998aa356"
			},
		//pre_p[8]
			{
				"d3f41432973b72ccdf63d92fdfd3b6cdf3a48784",
				"7a5fc61f45f32aa3862b51c0ffa391d55fb3ae8a"
			},
		//pre_p[9]
			{
				"c5ea261ba20a32ed86d80714a05d422a37ef08ef",
				"631d32bf43f2a08b3ebe08a11aad8eaecd2d5a30"
			},
		//pre_p[10]
			{
				"cb7a1c7938fff46bf494f0617d5b4120af2d9ef2",
				"9a39d38c7fbe1446bdafa81ea3e2df4801a64369"
			},
		//pre_p[11]
			{
				"2f791cd170f2a6d9022d8c8d2e08a376c45e9990",
				"6e3ac2adfb581d29412427ee717c42b3cd2318f6"
			},
		//pre_p[12]
			{
				"b2e28822ef4ea8c836d8689294d464224b6810d1",
				"24fee5577bb64462cbaf75c9792c78f9d19bb4"
			},
		//pre_p[13]
			{
				"9b71c7380cd6e54f7fdd567f26a35b77d6246af6",
				"536088fc52944be364749153a1edab9d8cf09138"
			},
		//pre_p[14]
			{
				"26551b261ce62e14448da975748d3455f2c53b26",
				"87d6ec74410706b8acdfbabe6549b280784f905b"
			},
		//pre_p[15]
			{
				"b64a4d06cb54d5aa8ca9616868233152bb9027a",
				"abe8e0e7003d847eb622a68525bdc78c4e33775d"
			},
		},
		//secp192r1
		{
			//pre_p[0]
			{
				"0",
				"0"
			},
		//pre_p[1]
			{
				"188da80eb03090f67cbf20eb43a18800f4ff0afd82ff1012",
				"7192b95ffc8da78631011ed6b24cdd573f977a11e794811"
			},
		//pre_p[2]
			{
				"67e30cbdcba671fbeb2cdfae5a927c35c39649c55d7c48d8",
				"a93549c45810f5c3ce32d03c063015777a83cee1ecbfbe7d"
			},
		//pre_p[3]
			{
				"ce216bb8452006e0f29e6feadfc9bf1a6f5ef88966e3ead3",
				"d98a2ee25aaec9581d0aeb4bb5b80a2046b9092d927b3779"
			},
		//pre_p[4]
			{
				"51a581d9184ac7374730d4f480d1090bb19963d8c0a1e340",
				"5bd81ee2e0bb9f6e7cdfcea02f683f16ecc56731e69912a5"
			},
		//pre_p[5]
			{
				"c730691d0f8dc240757eea7f292c341e4b15a2dd4f43374",
				"31b24c319354de3e008a9e83e9de8708df797890bbf45e00"
			},
		//pre_p[6]
			{
				"f0f4088361d24416c94c21d9f84f41e1cb5ec043ddf63aba",
				"27853c1ae13ea488e5de3b5b16bcd0caf37585b0406495f7"
			},
		//pre_p[7]
			{
				"611dea39716469f749e528eee29f7a9d074232a8e8ae68f",
				"f95460528338bdc96a6546543a727de666b867dd0d8043cc"
			},
		//pre_p[8]
			{
				"4bc1f71951f75131c245c02b904b596b6eb71930c5d8f50",
				"150096e7fd69f8d08bec948a8c21962aa4d0916ebe34803d"
			},
		//pre_p[9]
			{
				"9f56d96ceca3ba2a7d69a0b04322d065bd44ffe8e71aac0c",
				"a4e07fb335de73d9837d62ddc3f4575aee59f0d125a59dce"
			},
		//pre_p[10]
			{
				"e95ad210f3185c11ff549832a33d44b0ec76760c1cf46ae2",
				"a5ea86f68f3a27c851d39136b0ab1169273e5ec538ed372e"
			},
		//pre_p[11]
			{
				"da65e86285c120c953636ee56338e9b291237ea74d2d7d5",
				"26d65c2e3aa06272fc898ab9978391b21302f04cf13c3233"
			},
		//pre_p[12]
			{
				"91f199f2e5bbe86d45db23aefe113c6cd50947a818c5efe6",
				"eccd57e8ac8d8a19387343e9475daea4376881b660fec064"
			},
		//pre_p[13]
			{
				"2298f204ce6a83f9374c0a4c192ebcd6c9fef5b95b510228",
				"e713a40010c3c94906d58644efeb2cc046e4b820f4c574d0"
			},
		//pre_p[14]
			{
				"d873ced76cb7be1f27af7d5eb85e1f996178cb0e2d64eedf",
				"fc599808dc41f8e2a3d7b9576004d9adefc9129c52a67f9c"
			},
		//pre_p[15]
			{
				"faa9e432154e061cad167ef0d7f93bf4bb6c6b597fdf928a",
				"d877cdddd6ae3cf36101b2bed5f7e08f0c3d0d63d52c8f3f"
			},
		},
		//secp224r1
		{
			//pre_p[0]
			{
				"0",
				"0"
			},
		//pre_p[1]
			{
				"b70e0cbd6bb4bf7f321390b94a03c1d356c21122343280d6115c1d21",
				"bd376388b5f723fb4c22dfe6cd4375a05a07476444d5819985007e34"
			},
		//pre_p[2]
			{
				"1f49bbb0f99bc52242df8d8a2a43bca7664d40ce5efd9675666ebbe9",
				"9812dd4eb8d321dc858f185310d0ece8608436e66229e0b892dc9c43"
			},
		//pre_p[3]
			{
				"eedcccd8c2a74816e9a3bbce8a3f559eed1cb362f16d3e678d5d8eb8",
				"5cb379ba910a17313865468fafecabf2b4bf65f9dff19f90ed50266d"
			},
		//pre_p[4]
			{
				"499aa8a5f8ebeefec27a4e13a0b91fb2991fab0a00641966cab26e3",
				"6916f6d4338c5b81d77aae82f70684d929610d54507510407766af5d"
			},
		//pre_p[5]
			{
				"932b56be7685a3dd323ae4d1c8b1086000905e82d4ea95ac3b1f15c6",
				"523e80f019062afdec76dbe2a8a741665960f390f09ef93dea25dbbf"
			},
		//pre_p[6]
			{
				"c391b45c84725c363f37347c1ba4a01c83531b5d0f822fdd26732c73",
				"1efb7890ede244c393da7e222a7fddfbcde19dfaecbbd5e1b2d6ad24"
			},
		//pre_p[7]
			{
				"2610b39409f849ff8d33c2c98b7cd11beca79159bb4c9e90ca217da1",
				"fe67e4e820b112966c079b753c89cdbb7b2c46b4fb44d1352ac64da0"
			},
		//pre_p[8]
			{
				"5c73240899b4779b7619a3e7c4cc71b61d16f5c6ee28cae2df5312d",
				"72855882b08394641fab2116fd5618615165c56bda9f7f6382c73e3a"
			},
		//pre_p[9]
			{
				"41c98768fb0c1db89da93489a3e074a98ca8d00fb50469182f161c09",
				"e5e03408738a71cfe2d3fbf59e63dce9ffbca6855e5ea05fb32da81"
			},
		//pre_p[10]
			{
				"cb0c5d0dc34f24e03ab9f738beb84430137a5dd2f6dab22b2333e87f",
				"c40331df8938819281d688bcbe50185ba5c3fa2044764a7df0c8fda5"
			},
		//pre_p[11]
			{
				"1765bf22a5a9841a0c83fb4884daade92bd26909a3b89530796f0f60",
				"e2a215d96113694c1edba8b14e2f23bc6c67cec16f772a9ee75db09e"
			},
		//pre_p[12]
			{
				"6c7e4be83bbde3c8ae85fada74feade88696410552571e509fb5efb3",
				"286d2db3ee9a60a2946c53b582f4b47ce2495a6539ff9f51160f4652"
			},
		//pre_p[13]
			{
				"215619e9cc0057bcefba6f47f6d00995183b13926c40bbd5081a44af",
				"e7e3f4b0982db98631b93cdfe8b5f11c54a3694f6f8bc94d3b0df45e"
			},
		//pre_p[14]
			{
				"c813132f4c07e91c29819435d2c6ac38f36ff8a1d8b17048ab3e1c7b",
				"1e28ebf18562bcf5426ba5cc967408781030579fea2891425503b11f"
			},
		//pre_p[15]
			{
				"f1aef351497c58ff17036691a97306cd91d28b5e4c9f31997cc864eb",
				"eea7471f96255874a684435bd693dead073b1402dbdd1f2d600564ff"
			},
		},
		//secp256r1
		{
			//pre_p[0]
			{
				"0",
				"0"
			},
		//pre_p[1]
			{
				"6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",
				"4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5"
			},
		//pre_p[2]
			{
				"fa822bc2811aaa58492592e326e25de29493baaad651f7e90e75cb48e14db63",
				"bff44ae8f5dba80d6f4ad4bcb3df188b34b1a65050fe82f5e41124545f462ee7"
			},
		//pre_p[3]
			{
				"300a4bbc89d6726fb257c0de95e02789e96c98fd0d35f1fa93391ce2097992af",
				"72aac7e0d09b46447f1ddb25ff1e3c6f5bb1eeada9d806a5aa54a291c08127a0"
			},
		//pre_p[4]
			{
				"447d739beedb5e67fb982fd588c6766efc35ff7dc297eac357c84fc9d789bd85",
				"2d4825ab834131eee12e9d953a4aaff73d349b95a7fae5000c7e33c972e25b32"
			},
		//pre_p[5]
			{
				"ef9519328a9c72ffddc6068bb91dfc60ef7fbd2b1a0a11b713949c932a1d367f",
				"611e9fc37dbb2c9bc1ee9807022c219c23183b0895ca1740196035a77376d8a8"
			},
		//pre_p[6]
			{
				"550663797b51f5d87dea6482e11238bf2936df5ec6c9bc36cae2b1920b57f4bc",
				"157164848aecb8510afa40018d9d50e59fb3d576dbdefbe144ffe216348a964c"
			},
		//pre_p[7]
			{
				"eb5d7745b21141eaa2e8f483f43e43917ccd84e70d715f26e48ecafffc5cde01",
				"eafd72ebdbecc17b0990e6a158006cee85f22cfe2844b645cac917e2731a3479"
			},
		//pre_p[8]
			{
				"a6d39677a78492762736ff8344315fc596439591a3c6b94a6cf20ffb313728be",
				"674f84749b0b881666b8babd2d27ecdf824a920c2284059bf2bab833c357f5f4"
			},
		//pre_p[9]
			{
				"4e769e7672c9ddad31855f7db8c7fedb74e02f080203a56b2df48c04677c8a3e",
				"42b99082de8306631ec0057206947281fb9ae16f3b9122a5a4c36165b824bbb0"
			},
		//pre_p[10]
			{
				"78878ef61c6ce04d7fdc1ca008a1c478d1f89e799c0ce1316ef95150dda868b9",
				"b6cb3f5d7b72c321de53142c12309def6ace570ebde08d4f9c62b9121fe0d976"
			},
		//pre_p[11]
			{
				"c88bc4d716b1287595c5220812ffcae5b82dd5bd54fb4967f991ed2c31a3573",
				"dd5ddea3f3901dc618d1b5b39c04e6aa7c8181f4df2564f33a57bf635f48aca8"
			},
		//pre_p[12]
			{
				"68f344af6b317466efe0a423083e49f343a0a28c42ba792fe96a79fb3e72ad0c",
				"31b9c405f8540a20604ed93c24d67ff3668bfc2271f5c626cdfe17db3fb24d4a"
			},
		//pre_p[13]
			{
				"4052bf4b6f461db9663c62c3edbad7a00d1a10144ec39c28d36b4789a2582e7f",
				"fecf4d5190b0fc61862be6bd71d70cc8e724f33999bfcc5b235a27c3188d25eb"
			},
		//pre_p[14]
			{
				"1eddbae2c802e41a123202a8f62bff7aafdf5cc08526a7a474346c10a1d4cfac",
				"43104d86560ebcfc0c45f45273db33a036e06b7e4c7019178fa0af2dd603f844"
			},
		//pre_p[15]
			{
				"b48e26b484f7a21c0a4a46fb6aaf363a66b0de3225c4744b9615b5110d1d78e5",
				"fac015404d4d3dab64131bcdfed6f668c004e4048b7b0f9806ebb0f621a01b2d"
			}
		}
	};

	int i;
	if (ecname > 3)
	{
		return -1;
	}
	for (i = 0; i < 16; i++)
	{
		mpz_set_str(pre_p[i]->x, pre_cal_values[ecname][i][0], 16);
		mpz_set_str(pre_p[i]->y, pre_cal_values[ecname][i][1], 16);
		mpz_set_d(pre_p[i]->z, 1);
		mpz_set_d(pre_p[i]->zz, 1);
	}
	return 0;
}
