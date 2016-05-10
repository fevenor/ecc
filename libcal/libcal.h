/*
椭圆曲线群运算及标量乘运算库
提供点加、倍点，标量乘函数
*/
#include "malloc.h"
#if defined __GNUC__					//判断GCC环境
#include "gmp.h"						//GMP头文件
//gcc编译需要-lgmp选项
#elif defined _MSC_VER					//判断visual studio环境
#include "..\\mpir\\mpir.h"				//MPIR头文件
#pragma comment(lib, "..\\mpir\\mpir.lib")	//MPIR静态链接库
#endif

//affine坐标系的点
typedef struct affine_point
{
	mpz_t x;
	mpz_t y;
}af_p;
//jacobian坐标系的点
typedef struct jacobian_point
{
	mpz_t x;
	mpz_t y;
	mpz_t z;
	mpz_t zz;
}ja_p;


//初始化affine坐标系的点
extern af_p* af_p_inits();
//初始化affine坐标系的点并设置初值
extern af_p* af_p_inits_set_d(double x, double y);
//初始化jacobian坐标系的点
extern ja_p* ja_p_inits();

//释放affine坐标系的点占用的空间
extern void af_p_clears(af_p *p);
//释放jacobian坐标系的点占用的空间
extern void ja_p_clears(ja_p *p);

//将affine坐标系的点转换到jacobian坐标系
extern void af2ja(af_p *af, ja_p *ja);
//将jacobian坐标系的点转换到affine坐标系
extern void ja2af(mpz_t p, ja_p *ja, af_p *af);
//jacobian坐标系的点加运算
extern void point_add(mpz_t p, ja_p *p1, ja_p *p2, ja_p *p3);
//jacobian坐标系的倍点运算
extern void point_double(mpz_t p, ja_p *p1, ja_p *p3);
//设置jacobian坐标系上点的值
extern void point_set(ja_p *rp, ja_p *p);
//求-P(x,-y,z,zz)
extern void point_neg(ja_p *rp, ja_p *p);


//NAFw窗口法标量乘运算
extern void scalar_mul_w(int w, mpz_t k, ja_p *p, ja_p *q, mpz_t prime);