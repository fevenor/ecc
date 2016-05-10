/*
��Բ����Ⱥ���㼰�����������
�ṩ��ӡ����㣬�����˺���
*/
#include "malloc.h"
#if defined __GNUC__					//�ж�GCC����
#include "gmp.h"						//GMPͷ�ļ�
//gcc������Ҫ-lgmpѡ��
#elif defined _MSC_VER					//�ж�visual studio����
#include "..\\mpir\\mpir.h"				//MPIRͷ�ļ�
#pragma comment(lib, "..\\mpir\\mpir.lib")	//MPIR��̬���ӿ�
#endif

//affine����ϵ�ĵ�
typedef struct affine_point
{
	mpz_t x;
	mpz_t y;
}af_p;
//jacobian����ϵ�ĵ�
typedef struct jacobian_point
{
	mpz_t x;
	mpz_t y;
	mpz_t z;
	mpz_t zz;
}ja_p;


//��ʼ��affine����ϵ�ĵ�
extern af_p* af_p_inits();
//��ʼ��affine����ϵ�ĵ㲢���ó�ֵ
extern af_p* af_p_inits_set_d(double x, double y);
//��ʼ��jacobian����ϵ�ĵ�
extern ja_p* ja_p_inits();

//�ͷ�affine����ϵ�ĵ�ռ�õĿռ�
extern void af_p_clears(af_p *p);
//�ͷ�jacobian����ϵ�ĵ�ռ�õĿռ�
extern void ja_p_clears(ja_p *p);

//��affine����ϵ�ĵ�ת����jacobian����ϵ
extern void af2ja(af_p *af, ja_p *ja);
//��jacobian����ϵ�ĵ�ת����affine����ϵ
extern void ja2af(mpz_t p, ja_p *ja, af_p *af);
//jacobian����ϵ�ĵ������
extern void point_add(mpz_t p, ja_p *p1, ja_p *p2, ja_p *p3);
//jacobian����ϵ�ı�������
extern void point_double(mpz_t p, ja_p *p1, ja_p *p3);
//����jacobian����ϵ�ϵ��ֵ
extern void point_set(ja_p *rp, ja_p *p);
//��-P(x,-y,z,zz)
extern void point_neg(ja_p *rp, ja_p *p);


//NAFw���ڷ�����������
extern void scalar_mul_w(int w, mpz_t k, ja_p *p, ja_p *q, mpz_t prime);