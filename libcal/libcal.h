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

typedef struct affine_point		//affine����ϵ�ĵ�
{
	mpz_t x;
	mpz_t y;
}af_p;

typedef struct jacobian_point	//jacobian����ϵ�ĵ�
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
extern int af2ja(af_p *af, ja_p *ja);
//��jacobian����ϵ�ĵ�ת����affine����ϵ
extern int ja2af(mpz_t p, ja_p *ja, af_p *af);
//jacobian����ϵ�ĵ������
extern int point_add(mpz_t p, ja_p *p1, ja_p *p2, ja_p *p3);
//jacobian����ϵ�ı�������
extern int point_double(mpz_t p, ja_p *p1, ja_p *p3);

