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

void pre_cal(enum curve_name ecname, ja_p *pre_p[][16])
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

	//table0
	//0000
	mpz_set_d(pre_p[0][0]->z, 1);
	mpz_set_d(pre_p[0][0]->zz, 1);
	//0001
	point_set(pre_p[0][1], c->g);
	//0010
	point_set(pre_p[0][2], pre_p[0][1]);
	for (i = 0; i < k_c_length; i++)
	{
		point_double(c->p, pre_p[0][2], pre_p[0][2]);
	}
	//0011
	point_add(c->p, pre_p[0][1], pre_p[0][2], pre_p[0][3]);
	//0100
	point_set(pre_p[0][4], pre_p[0][2]);
	for (i = 0; i < k_c_length; i++)
	{
		point_double(c->p, pre_p[0][4], pre_p[0][4]);
	}
	//0101
	point_add(c->p, pre_p[0][1], pre_p[0][4], pre_p[0][5]);
	//0110
	point_add(c->p, pre_p[0][2], pre_p[0][4], pre_p[0][6]);
	//0111
	point_add(c->p, pre_p[0][1], pre_p[0][6], pre_p[0][7]);
	//1000
	point_set(pre_p[0][8], pre_p[0][4]);
	for (i = 0; i < k_c_length; i++)
	{
		point_double(c->p, pre_p[0][8], pre_p[0][8]);
	}
	//1001
	point_add(c->p, pre_p[0][1], pre_p[0][8], pre_p[0][9]);
	//1010
	point_add(c->p, pre_p[0][2], pre_p[0][8], pre_p[0][10]);
	//1011
	point_add(c->p, pre_p[0][1], pre_p[0][10], pre_p[0][11]);
	//1100
	point_add(c->p, pre_p[0][4], pre_p[0][8], pre_p[0][12]);
	//1101
	point_add(c->p, pre_p[0][1], pre_p[0][12], pre_p[0][13]);
	//1110
	point_add(c->p, pre_p[0][2], pre_p[0][12], pre_p[0][14]);
	//1111
	point_add(c->p, pre_p[0][1], pre_p[0][14], pre_p[0][15]);

	//table1
	//0000
	mpz_set_d(pre_p[1][0]->z, 1);
	mpz_set_d(pre_p[1][0]->zz, 1);
	//0001
	point_set(pre_p[1][1], c->g);
	for (i = 0; i < k_c_length / 2; i++)
	{
		point_double(c->p, pre_p[1][1], pre_p[1][1]);
	}
	//0010
	point_set(pre_p[1][2], pre_p[1][1]);
	for (i = 0; i < k_c_length; i++)
	{
		point_double(c->p, pre_p[1][2], pre_p[1][2]);
	}
	//0011
	point_add(c->p, pre_p[1][1], pre_p[1][2], pre_p[1][3]);
	//0100
	point_set(pre_p[1][4], pre_p[1][2]);
	for (i = 0; i < k_c_length; i++)
	{
		point_double(c->p, pre_p[1][4], pre_p[1][4]);
	}
	//0101
	point_add(c->p, pre_p[1][1], pre_p[1][4], pre_p[1][5]);
	//0110
	point_add(c->p, pre_p[1][2], pre_p[1][4], pre_p[1][6]);
	//0111
	point_add(c->p, pre_p[1][1], pre_p[1][6], pre_p[1][7]);
	//1000
	point_set(pre_p[1][8], pre_p[1][4]);
	for (i = 0; i < k_c_length; i++)
	{
		point_double(c->p, pre_p[1][8], pre_p[1][8]);
	}
	//1001
	point_add(c->p, pre_p[1][1], pre_p[1][8], pre_p[1][9]);
	//1010
	point_add(c->p, pre_p[1][2], pre_p[1][8], pre_p[1][10]);
	//1011
	point_add(c->p, pre_p[1][1], pre_p[1][10], pre_p[1][11]);
	//1100
	point_add(c->p, pre_p[1][4], pre_p[1][8], pre_p[1][12]);
	//1101
	point_add(c->p, pre_p[1][1], pre_p[1][12], pre_p[1][13]);
	//1110
	point_add(c->p, pre_p[1][2], pre_p[1][12], pre_p[1][14]);
	//1111
	point_add(c->p, pre_p[1][1], pre_p[1][14], pre_p[1][15]);


	//Î¨Ò»»¯
	for (i = 0; i < 16; i++)
	{
		ja2af(c->p, pre_p[0][i], temp);
		af2ja(temp, pre_p[0][i]);
		ja2af(c->p, pre_p[1][i], temp);
		af2ja(temp, pre_p[1][i]);
	}

}

int get_pre_cal_value(enum curve_name ecname, ja_p *pre_p[][16])
{
	char pre_cal_values[][2][16][2][65] =
	{
		//secp160r2
		{
			{
				//pre_p[0][1]
				{
					"0",
					"0"
				},
			//pre_p[0][1]
				{
					"52dcb034293a117e1f4ff11b30f7199d3144ce6d",
					"feaffef2e331f296e071fa0df9982cfea7d43f2e"
				},
			//pre_p[0][2]
				{
					"3814a549594c44ce6caea79745742ea589abdd96",
					"5b0a002d8e794b12cfb2ff8a2eaf75ac3ffe95d4"
				},
			//pre_p[0][3]
				{
					"c74b5cdd6c896d1399fc66513e49592b23cfd102",
					"1f6c85e2b2275b985df7d96582405fd505aa04f5"
				},
			//pre_p[0][4]
				{
					"af6092be029e4d9b06a1961f2dd8da5d9fdbe7cc",
					"3e9221cddcd3fb29952f7d3556d0ee4731a7c90d"
				},
			//pre_p[0][5]
				{
					"2a9f47a18c7f44dadadbb9346222cbb159aa3451",
					"b78c590839ffbb73511c79a14833968a23687833"
				},
			//pre_p[0][6]
				{
					"8d4a4a4c748b0fbc8f97d00419fee8fe876283a2",
					"d94e3c0416fe54e443bc096d12849a187b4dfb9d"
				},
			//pre_p[0][7]
				{
					"f2b642273aaca989f9028434b489efa1dd6eb53a",
					"a4c3839b5a6d6d5f314068728a059262998aa356"
				},
			//pre_p[0][8]
				{
					"d3f41432973b72ccdf63d92fdfd3b6cdf3a48784",
					"7a5fc61f45f32aa3862b51c0ffa391d55fb3ae8a"
				},
			//pre_p[0][9]
				{
					"c5ea261ba20a32ed86d80714a05d422a37ef08ef",
					"631d32bf43f2a08b3ebe08a11aad8eaecd2d5a30"
				},
			//pre_p[0][10]
				{
					"cb7a1c7938fff46bf494f0617d5b4120af2d9ef2",
					"9a39d38c7fbe1446bdafa81ea3e2df4801a64369"
				},
			//pre_p[0][11]
				{
					"2f791cd170f2a6d9022d8c8d2e08a376c45e9990",
					"6e3ac2adfb581d29412427ee717c42b3cd2318f6"
				},
			//pre_p[0][12]
				{
					"b2e28822ef4ea8c836d8689294d464224b6810d1",
					"24fee5577bb64462cbaf75c9792c78f9d19bb4"
				},
			//pre_p[0][13]
				{
					"9b71c7380cd6e54f7fdd567f26a35b77d6246af6",
					"536088fc52944be364749153a1edab9d8cf09138"
				},
			//pre_p[0][14]
				{
					"26551b261ce62e14448da975748d3455f2c53b26",
					"87d6ec74410706b8acdfbabe6549b280784f905b"
				},
			//pre_p[0][15]
				{
					"b64a4d06cb54d5aa8ca9616868233152bb9027a",
					"abe8e0e7003d847eb622a68525bdc78c4e33775d"
				}
			},
			{
				//pre_p[1][0]
				{
					"0",
					"0"
				},
					//pre_p[1][1]
				{
					"1cb0d0dc3939b589518d12665c8a069f39e15351",
					"d6a764441e85e3b5b3a4784c68b1edbb10586923"
				},
						//pre_p[1][2]
				{
					"7edf42b67b7b136106fda9dba3f5c02f8a416708",
					"b4b09b0e71996103bdc31c5818d27e69baf462d6"
				},
						//pre_p[1][3]
				{
					"d8bcb9485103b7b88d1e4ab3870c37b35ab93bcf",
					"a506d509c7f12a90e29f9da955a76de03b5673ff"
				},
						//pre_p[1][4]
				{
					"eb10eb00bbfe561bcf5712249918fe702a09f424",
					"6848ea4dd3a8c053dc3e633b69bd4771f322cd6d"
				},
						//pre_p[1][5]
				{
					"682fa7d529f813044338111f831e037ad6b8d2fd",
					"b4797830632741e58a814576035184f4e3ec4b13"
				},
						//pre_p[1][6]
				{
					"731626493864ae072269e6c358c3ed4d752a0d35",
					"4330087b854fec03e1d9a3b96f477768b4bf96e6"
				},
						//pre_p[1][7]
				{
					"9abdf2c9bdca721f0aa3bc426525f98f705ca548",
					"87df1eb64ac5c7854362bed0281aa480d17bc898"
				},
						//pre_p[1][8]
				{
					"c01fdeb5f8e4d21fe325b0e3067d8087c1a930a1",
					"53f692cb62d0a585870996ba5b67bbe28a61418f"
				},
						//pre_p[1][9]
				{
					"6fb0769f8be415b5fdcf974095abfa845cb7df1a",
					"c22ea8b9a2f19db0673819dcc43e5697a4a98ed3"
				},
						//pre_p[1][10]
				{
					"98e6ab8aac25dc89703a23eec5a36d9e6c0f2bfa",
					"92284d53f18244424e3a593c91d4437cdd890cd1"
				},
						//pre_p[1][11]
				{
					"c0bf2a352c94390a2d095055b61f4db70fbac2e9",
					"c2d82eadfbd8915b54b9e03d37d8de012a05b028"
				},
						//pre_p[1][12]
				{
					"4bfebb2eaa34c591bf7d4ae6f195bcf47d9f542e",
					"44ca557c63e03e5eb33e8177c5016ba6f338e1d0"
				},
						//pre_p[1][13]
				{
					"8c47c00357c02094930c7f5a16495776302cd1a5",
					"9146b639278110f21a69a04258e47517322db820"
				},
						//pre_p[1][14]
				{
					"3f6509b0aac93ca8a7cacc718f90817e9965590b",
					"92d356acb8fd6db5cf15e0d9d08a0a94af0381a9"
				},
						//pre_p[1][15]
				{
					"dcce50a6be13bd81e02a474f67aacd78609cbd3b",
					"45ea66083a46f71bf2e117536d0b565907dc3a34"
				}
			}
		},
		//secp192r1
		{
			{
				//pre_p[0][0]
				{
					"0",
					"0"
				},
			//pre_p[0][1]
				{
					"188da80eb03090f67cbf20eb43a18800f4ff0afd82ff1012",
					"7192b95ffc8da78631011ed6b24cdd573f977a11e794811"
				},
			//pre_p[0][2]
				{
					"67e30cbdcba671fbeb2cdfae5a927c35c39649c55d7c48d8",
					"a93549c45810f5c3ce32d03c063015777a83cee1ecbfbe7d"
				},
			//pre_p[0][3]
				{
					"ce216bb8452006e0f29e6feadfc9bf1a6f5ef88966e3ead3",
					"d98a2ee25aaec9581d0aeb4bb5b80a2046b9092d927b3779"
				},
			//pre_p[0][4]
				{
					"51a581d9184ac7374730d4f480d1090bb19963d8c0a1e340",
					"5bd81ee2e0bb9f6e7cdfcea02f683f16ecc56731e69912a5"
				},
			//pre_p[0][5]
				{
					"c730691d0f8dc240757eea7f292c341e4b15a2dd4f43374",
					"31b24c319354de3e008a9e83e9de8708df797890bbf45e00"
				},
			//pre_p[0][6]
				{
					"f0f4088361d24416c94c21d9f84f41e1cb5ec043ddf63aba",
					"27853c1ae13ea488e5de3b5b16bcd0caf37585b0406495f7"
				},
			//pre_p[0][7]
				{
					"611dea39716469f749e528eee29f7a9d074232a8e8ae68f",
					"f95460528338bdc96a6546543a727de666b867dd0d8043cc"
				},
			//pre_p[0][8]
				{
					"4bc1f71951f75131c245c02b904b596b6eb71930c5d8f50",
					"150096e7fd69f8d08bec948a8c21962aa4d0916ebe34803d"
				},
			//pre_p[0][9]
				{
					"9f56d96ceca3ba2a7d69a0b04322d065bd44ffe8e71aac0c",
					"a4e07fb335de73d9837d62ddc3f4575aee59f0d125a59dce"
				},
			//pre_p[0][10]
				{
					"e95ad210f3185c11ff549832a33d44b0ec76760c1cf46ae2",
					"a5ea86f68f3a27c851d39136b0ab1169273e5ec538ed372e"
				},
			//pre_p[0][11]
				{
					"da65e86285c120c953636ee56338e9b291237ea74d2d7d5",
					"26d65c2e3aa06272fc898ab9978391b21302f04cf13c3233"
				},
			//pre_p[0][12]
				{
					"91f199f2e5bbe86d45db23aefe113c6cd50947a818c5efe6",
					"eccd57e8ac8d8a19387343e9475daea4376881b660fec064"
				},
			//pre_p[0][13]
				{
					"2298f204ce6a83f9374c0a4c192ebcd6c9fef5b95b510228",
					"e713a40010c3c94906d58644efeb2cc046e4b820f4c574d0"
				},
			//pre_p[0][14]
				{
					"d873ced76cb7be1f27af7d5eb85e1f996178cb0e2d64eedf",
					"fc599808dc41f8e2a3d7b9576004d9adefc9129c52a67f9c"
				},
			//pre_p[0][15]
				{
					"faa9e432154e061cad167ef0d7f93bf4bb6c6b597fdf928a",
					"d877cdddd6ae3cf36101b2bed5f7e08f0c3d0d63d52c8f3f"
				}
			},
			{
				//pre_p[1][0]
				{
					"0",
					"0"
				},
			//pre_p[1][1]
				{
					"9b95f940bd263b56b80ec5b18b038fdf60bd792ca732037",
					"7d80d527355ed561fbc3c982043f87f319ec93700aec1acd"
				},
			//pre_p[1][2]
				{
					"bd12ebd12ae54d508bee2c1d00914a56e9a045f1b2b87be3",
					"ac2bd78d9e7d57b3ecc58fafe7ce8f40b283a79315349181"
				},
			//pre_p[1][3]
				{
					"f9b6b217f6e1da455d468921b741b07a438fd6393a236e7f",
					"fbdd829559f6d53e8ecf5e642fcb8eb9010881800f912f77"
				},
			//pre_p[1][4]
				{
					"4af48c159ceac819ebedd123e2035aee792fb6b77e09a7bf",
					"b0d449d8752be821d0be4cdbb3fcec631b22e942a287bce4"
				},
			//pre_p[1][5]
				{
					"d9c28690eea959ceb3b7d99a9bc0d6b56dbea2ef1befd43f",
					"b2b755fb6ff08892f10251f8097b930eb3772631ad61fda8"
				},
			//pre_p[1][6]
				{
					"2a30d4f02f908e6e338c1037c8c1c699738609a197aa8faf",
					"1df16a4b67ecedd71abd7401b1f949f7590a5abcd15a4833"
				},
			//pre_p[1][7]
				{
					"22d33581f4c1afcc925371c8e280813214f843e1bbc86493",
					"ef940bf9cf492393aad243f1d53615e20fe566c7e6062bdf"
				},
			//pre_p[1][8]
				{
					"8eb0322c42165379f7a98167a1a0d1ce720566990dd81268",
					"89f9b030eecccca60b7e8cc2b830d8d3b58f57dd7aa1c0a5"
				},
			//pre_p[1][9]
				{
					"cd551b0da4dd18b9d38b5393f4bd0068227c3306fb693194",
					"e302d7afffacb1ab1f3224d19b2e712933e7149ece2b5212"
				},
			//pre_p[1][10]
				{
					"8b01aa397cf6a974d7706ae8d79f7d882db266d1b8a28a1",
					"3302d751d93555f1d54fb42e0eef12f082d86bf3298c60d2"
				},
			//pre_p[1][11]
				{
					"e2c851938183059ae0bcce2b04480a8cfde819275fa25263",
					"a5ea13eb57f9a32a92ab53487ccf5e0301453a848b15ff5e"
				},
			//pre_p[1][12]
				{
					"5e55b54bb877d22aa52989e25c3a29e054993872a67d4d23",
					"1e666394c56aab4b974cbb0c16a11d78cb3b37971f7bb78"
				},
			//pre_p[1][13]
				{
					"28cf582703d2dd7d59f278e444a3ba56767a68a281aa4893",
					"2775d431e2d9ead94b83ffc0ddc85c32d2de77a7865186bb"
				},
			//pre_p[1][14]
				{
					"2622bb9bb370a85890b76d3f03c18332a82e5a4522efa6c0",
					"81a394de791c9f98fb67fe9346215a629b69f7fde83cb02d"
				},
			//pre_p[1][15]
				{
					"2ff0ba56f03b1347c936631f11bec1f0dc6d4ee566006bab",
					"887d9d4a3be85d76582b90de708ee90a52fa722169a676be"
				}
			}
		},
		//secp224r1
		{
			{
				//pre_p[0][0]
				{
					"0",
					"0"
				},
			//pre_p[0][1]
				{
					"b70e0cbd6bb4bf7f321390b94a03c1d356c21122343280d6115c1d21",
					"bd376388b5f723fb4c22dfe6cd4375a05a07476444d5819985007e34"
				},
			//pre_p[0][2]
				{
					"1f49bbb0f99bc52242df8d8a2a43bca7664d40ce5efd9675666ebbe9",
					"9812dd4eb8d321dc858f185310d0ece8608436e66229e0b892dc9c43"
				},
			//pre_p[0][3]
				{
					"eedcccd8c2a74816e9a3bbce8a3f559eed1cb362f16d3e678d5d8eb8",
					"5cb379ba910a17313865468fafecabf2b4bf65f9dff19f90ed50266d"
				},
			//pre_p[0][4]
				{
					"499aa8a5f8ebeefec27a4e13a0b91fb2991fab0a00641966cab26e3",
					"6916f6d4338c5b81d77aae82f70684d929610d54507510407766af5d"
				},
			//pre_p[0][5]
				{
					"932b56be7685a3dd323ae4d1c8b1086000905e82d4ea95ac3b1f15c6",
					"523e80f019062afdec76dbe2a8a741665960f390f09ef93dea25dbbf"
				},
			//pre_p[0][6]
				{
					"c391b45c84725c363f37347c1ba4a01c83531b5d0f822fdd26732c73",
					"1efb7890ede244c393da7e222a7fddfbcde19dfaecbbd5e1b2d6ad24"
				},
			//pre_p[0][7]
				{
					"2610b39409f849ff8d33c2c98b7cd11beca79159bb4c9e90ca217da1",
					"fe67e4e820b112966c079b753c89cdbb7b2c46b4fb44d1352ac64da0"
				},
			//pre_p[0][8]
				{
					"5c73240899b4779b7619a3e7c4cc71b61d16f5c6ee28cae2df5312d",
					"72855882b08394641fab2116fd5618615165c56bda9f7f6382c73e3a"
				},
			//pre_p[0][9]
				{
					"41c98768fb0c1db89da93489a3e074a98ca8d00fb50469182f161c09",
					"e5e03408738a71cfe2d3fbf59e63dce9ffbca6855e5ea05fb32da81"
				},
			//pre_p[0][10]
				{
					"cb0c5d0dc34f24e03ab9f738beb84430137a5dd2f6dab22b2333e87f",
					"c40331df8938819281d688bcbe50185ba5c3fa2044764a7df0c8fda5"
				},
			//pre_p[0][11]
				{
					"1765bf22a5a9841a0c83fb4884daade92bd26909a3b89530796f0f60",
					"e2a215d96113694c1edba8b14e2f23bc6c67cec16f772a9ee75db09e"
				},
			//pre_p[0][12]
				{
					"6c7e4be83bbde3c8ae85fada74feade88696410552571e509fb5efb3",
					"286d2db3ee9a60a2946c53b582f4b47ce2495a6539ff9f51160f4652"
				},
			//pre_p[0][13]
				{
					"215619e9cc0057bcefba6f47f6d00995183b13926c40bbd5081a44af",
					"e7e3f4b0982db98631b93cdfe8b5f11c54a3694f6f8bc94d3b0df45e"
				},
			//pre_p[0][14]
				{
					"c813132f4c07e91c29819435d2c6ac38f36ff8a1d8b17048ab3e1c7b",
					"1e28ebf18562bcf5426ba5cc967408781030579fea2891425503b11f"
				},
			//pre_p[0][15]
				{
					"f1aef351497c58ff17036691a97306cd91d28b5e4c9f31997cc864eb",
					"eea7471f96255874a684435bd693dead073b1402dbdd1f2d600564ff"
				}
			},
			{
				//pre_p[1][0]
				{
					"0",
					"0"
				},
			//pre_p[1][1]
				{
					"d35ab74d6afc31ce27a34cdba4179613d78b60ef2d9665266dddf554",
					"355a1830e90f2da141cffd8c93c62137e5783a6aab85ccdd22deb15e"
				},
			//pre_p[1][2]
				{
					"65c3553c6090e3e7992996abec86d6da4da77fe53c1a494eadaade65",
					"2995b1b18c28ac51a13ccd3cbabf1c6540b8a4aaffa610b1fb09346"
				},
			//pre_p[1][3]
				{
					"d3966a44beac33dc0690a7550d9a86b419fbe38d047874568e7295ef",
					"d25e2513e3a100dc7dd88ae41200beaa3b6a032df32b7280ec29132f"
				},
			//pre_p[1][4]
				{
					"825800fd3562d58edaa1445553fcac2bce41223190924857eb2efafd",
					"d8db0cc277daeaaf8b219f9416b523a01c3dd9ed8d8d79148ea96621"
				},
			//pre_p[1][5]
				{
					"6322e97fe154be69212d1a6b0327e9acd29bc7e69176a9c3b1a700f0",
					"e4fcbe9325be511f8eae66c52b888d41ed18883b05469fc5465d62aa"
				},
			//pre_p[1][6]
				{
					"14ffd0a2daf2f1683c17744b0165020b857c7b023a825fdf583cac16",
					"bced4b01a28bbc15b3080841acf4944ec4e3b47d4323b36184218f9"
				},
			//pre_p[1][7]
				{
					"40064f2ba65233cb3f19870c0c9352f33b4063eda892ac22230df5c4",
					"6f8bc76760def1a57bb24f723a6012da25af5b517fe16f0924f8992"
				},
			//pre_p[1][8]
				{
					"dc0fe90e0f1aae3ec11e11d9c326bcab0738ee9a784a7084f7817cb9",
					"956ec2d617fc459afae98a4047b75c350aa22ffb74cf639ea5f98390"
				},
			//pre_p[1][9]
				{
					"bd1c3caff1a52ef5595e377d2f2e9247cd8bc9a4624306d648c1be6a",
					"191837685cdfbb745a602b2d4f7d29f3e17078f773045e14472409d0"
				},
			//pre_p[1][10]
				{
					"c4c225785c09dee21faeb1d1def44953433f5e70265b6ee254a8cb79",
					"c765866e33b9f347e91868839e8f31b125b1036db8307ce7bba1e518"
				},
			//pre_p[1][11]
				{
					"297ecd89714b05de5df64f95db264794da641e50933bfece24f96906",
					"9895089d66fe5813c5665658af167073b4f53cb1d5701bd3ebb2c3aa"
				},
			//pre_p[1][12]
				{
					"bbbd70df4911bb94229c3a951c942d773633b05d2e0fef05f78c4790",
					"7b7e94b39e67b09fdf6111614080105f47a72b0d73b2c6963d2c1168"
				},
			//pre_p[1][13]
				{
					"40d7558d7aa4d96b3bdf12438345f012482c0da69dad1a7d6efbe2b3",
					"dcd5f59f63c3ea5973f15f4f9b1c9a356e5d9ffd388a09fffb5c6d3d"
				},
			//pre_p[1][14]
				{
					"acd4644c9abbac64e3602cd11844c8071cc5fd737acf39f4c5ca7ab",
					"50b204ced940519f6f56574fad8fecd87ba24e32a6c011a36d8bf6e"
				},
			//pre_p[1][15]
				{
					"7405783dc3b55e778e4a3bf3ef9b5ceef7ad94c40aed4f1cae7d9a96",
					"3df90fbc4c7d0e91176d0a7e95d1b46a97570f018b32477c61b6e8c6"
				}
			}
		},
		//secp256r1
		{
			{
				//pre_p[0][0]
				{
					"0",
					"0"
				},
			//pre_p[0][1]
				{
					"6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",
					"4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5"
				},
			//pre_p[0][2]
				{
					"fa822bc2811aaa58492592e326e25de29493baaad651f7e90e75cb48e14db63",
					"bff44ae8f5dba80d6f4ad4bcb3df188b34b1a65050fe82f5e41124545f462ee7"
				},
			//pre_p[0][3]
				{
					"300a4bbc89d6726fb257c0de95e02789e96c98fd0d35f1fa93391ce2097992af",
					"72aac7e0d09b46447f1ddb25ff1e3c6f5bb1eeada9d806a5aa54a291c08127a0"
				},
			//pre_p[0][4]
				{
					"447d739beedb5e67fb982fd588c6766efc35ff7dc297eac357c84fc9d789bd85",
					"2d4825ab834131eee12e9d953a4aaff73d349b95a7fae5000c7e33c972e25b32"
				},
			//pre_p[0][5]
				{
					"ef9519328a9c72ffddc6068bb91dfc60ef7fbd2b1a0a11b713949c932a1d367f",
					"611e9fc37dbb2c9bc1ee9807022c219c23183b0895ca1740196035a77376d8a8"
				},
			//pre_p[0][6]
				{
					"550663797b51f5d87dea6482e11238bf2936df5ec6c9bc36cae2b1920b57f4bc",
					"157164848aecb8510afa40018d9d50e59fb3d576dbdefbe144ffe216348a964c"
				},
			//pre_p[0][7]
				{
					"eb5d7745b21141eaa2e8f483f43e43917ccd84e70d715f26e48ecafffc5cde01",
					"eafd72ebdbecc17b0990e6a158006cee85f22cfe2844b645cac917e2731a3479"
				},
			//pre_p[0][8]
				{
					"a6d39677a78492762736ff8344315fc596439591a3c6b94a6cf20ffb313728be",
					"674f84749b0b881666b8babd2d27ecdf824a920c2284059bf2bab833c357f5f4"
				},
			//pre_p[0][9]
				{
					"4e769e7672c9ddad31855f7db8c7fedb74e02f080203a56b2df48c04677c8a3e",
					"42b99082de8306631ec0057206947281fb9ae16f3b9122a5a4c36165b824bbb0"
				},
			//pre_p[0][10]
				{
					"78878ef61c6ce04d7fdc1ca008a1c478d1f89e799c0ce1316ef95150dda868b9",
					"b6cb3f5d7b72c321de53142c12309def6ace570ebde08d4f9c62b9121fe0d976"
				},
			//pre_p[0][11]
				{
					"c88bc4d716b1287595c5220812ffcae5b82dd5bd54fb4967f991ed2c31a3573",
					"dd5ddea3f3901dc618d1b5b39c04e6aa7c8181f4df2564f33a57bf635f48aca8"
				},
			//pre_p[0][12]
				{
					"68f344af6b317466efe0a423083e49f343a0a28c42ba792fe96a79fb3e72ad0c",
					"31b9c405f8540a20604ed93c24d67ff3668bfc2271f5c626cdfe17db3fb24d4a"
				},
			//pre_p[0][13]
				{
					"4052bf4b6f461db9663c62c3edbad7a00d1a10144ec39c28d36b4789a2582e7f",
					"fecf4d5190b0fc61862be6bd71d70cc8e724f33999bfcc5b235a27c3188d25eb"
				},
			//pre_p[0][14]
				{
					"1eddbae2c802e41a123202a8f62bff7aafdf5cc08526a7a474346c10a1d4cfac",
					"43104d86560ebcfc0c45f45273db33a036e06b7e4c7019178fa0af2dd603f844"
				},
			//pre_p[0][15]
				{
					"b48e26b484f7a21c0a4a46fb6aaf363a66b0de3225c4744b9615b5110d1d78e5",
					"fac015404d4d3dab64131bcdfed6f668c004e4048b7b0f9806ebb0f621a01b2d"
				}
			},
			{
				//pre_p[1][0]
				{
					"0",
					"0"
				},
			//pre_p[1][1]
				{
					"7fe36b40af22af8921656b32262c71da1ab919365c65dfb63a5a9e22185a5943",
					"e697d45825b636249f09f40407dca6f174b3d5867b8af212d50d152c699ca101"
				},
			//pre_p[1][2]
				{
					"4a5b506612a677a657880b3a18a2e902e9a521b074ca0141a84aa9397512218e",
					"eb13461ceac089f1c42604fbe1627d40626db15419e26d9d0beada7a4c4f3840"
				},
			//pre_p[1][3]
				{
					"781b8291c6a220ac342967aa815c8575e52c4144103ecbcf9faed0927a43281",
					"690cde8df015159397b2a14f1291643488f80eeee54a05e35a8343ceeac55f80"
				},
			//pre_p[1][4]
				{
					"8a535f566ec73617f5622df4373713269e4c35874afdf43aaee9c75df7f82f2a",
					"455c08468b08bd737e02819085a92bfcde533864c8c7669c5f9a0ac223094b7"
				},
			//pre_p[1][5]
				{
					"6bada7ab77f82765050a949b6cdc279f9a4bf62876dc4440c0a6e2c9477b5d9",
					"5b476dfd0e6cb18a427d49101366eb70debd8a4b7ea1070fc8b4aed1ea48dac9"
				},
			//pre_p[1][6]
				{
					"3e29864e8a2ec90829a751b1ae23c5d84d54606812d66f3b7c5c3e44278c340a",
					"239b90ea3dc31e7e1f150e68e322d1edad1744c4765bd780142d2a6626dbb850"
				},
			//pre_p[1][7]
				{
					"820f4dd949f72ff7dbcab759f8862ed4305dde6709776f8e78c416527a53322a",
					"140406ec783a05ec1b481b1b215c14d375be5d937b4e8cc46cc544a62b5debd4"
				},
			//pre_p[1][8]
				{
					"68f6b8542783dfeeeb5b06e70ce08ffefd75f3fa01876bd86a703f10e895df07",
					"cbe1feba92e40ce6fbc8044dfda45028cf5293d2f310bf7f90c76f8a78712655"
				},
			//pre_p[1][9]
				{
					"d0b2f94d2f420109230f729f2250e927fc82ef0b6acea274e998ceea4396e4c1",
					"971459828b0719e57db2636658954e7a10b838f8624c3b454305adddb38d4966"
				},
			//pre_p[1][10]
				{
					"961610004a866abac2d5cba4f234068757f2929e53d0b8764bd6b72623369fc9",
					"7acb9fadcee75e442cf1f2438fe5131c69ab197d92ddcb2449997bcd2e407a5e"
				},
			//pre_p[1][11]
				{
					"24eb9acca333bf5ba60d880f6f75aaeaf57f0c917aea685b254e839423d2d4c0",
					"69f891c5acd079cc743125f88bac4c4dfeef9341c51a6b4fe3de4ccb1cda5dea"
				},
			//pre_p[1][12]
				{
					"e51f547c5972a107b422d1e7bd6f85147ed031a0e45c2258eee44b35702476b5",
					"1c309a2b25bb1387a62f98b3a9fe9a068ca922ee097c184ea25bcd6fc9cf343d"
				},
			//pre_p[1][13]
				{
					"20b87b8aa2c4e503c504977708011828b00148833472c98e9295dbeb1967c459",
					"f5c6fa49919776be0d11adef5f69a0441bd539338fe582dd3063175de057c277"
				},
			//pre_p[1][14]
				{
					"1ed7d1b9332010b9a454c3fad83faa563876cba1102fad5f8c944e760fd59e11",
					"3a2b03f03217257a52b520f0eb6a2a2405e4d0dcac0cd344a1011a270024b889"
				},
			//pre_p[1][15]
				{
					"15fee545c78dd9f6fc7d229ca0058c3bf330240db58d5a62f20fc2afdf1d043d",
					"4ab5b6b2b8753f81557d9f49461210fb41ef80e5d046ac04501e82885bc98cda"
				}
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
		mpz_set_str(pre_p[0][i]->x, pre_cal_values[ecname][0][i][0], 16);
		mpz_set_str(pre_p[0][i]->y, pre_cal_values[ecname][0][i][1], 16);
		mpz_set_d(pre_p[0][i]->z, 1);
		mpz_set_d(pre_p[0][i]->zz, 1);
		mpz_set_str(pre_p[1][i]->x, pre_cal_values[ecname][1][i][0], 16);
		mpz_set_str(pre_p[1][i]->y, pre_cal_values[ecname][1][i][1], 16);
		mpz_set_d(pre_p[1][i]->z, 1);
		mpz_set_d(pre_p[1][i]->zz, 1);
	}
	return 0;
}
