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
//椭圆曲线群参数
typedef struct curve_group
{
	mpz_t p;
	mpz_t a;
	mpz_t b;
	ja_p *g;
	mpz_t n;
	int length;
}group;
//椭圆曲线名
enum curve_name
{
	secp160r2,
	secp192r1,
	secp224r1,
	secp256r1
};

//初始化affine坐标系的点
extern af_p* af_p_inits();
//初始化affine坐标系的点并设置初值
extern af_p* af_p_inits_set_d(double x, double y);
//初始化jacobian坐标系的点
extern ja_p* ja_p_inits();

//初始化椭圆曲线群参数
extern group* group_inits();

//释放affine坐标系的点占用的空间
extern void af_p_clears(af_p *p);
//释放jacobian坐标系的点占用的空间
extern void ja_p_clears(ja_p *p);
//释放椭圆曲线群参数占用的空间
extern void group_clears(group *c);

//将affine坐标系的点转换到jacobian坐标系
extern void af2ja(af_p *af, ja_p *ja);
//将jacobian坐标系的点转换到affine坐标系
extern void ja2af(mpz_t p, ja_p *ja, af_p *af);
//jacobian坐标系的点加运算
//p1+p2=p3
extern void point_add(mpz_t p, ja_p *p1, ja_p *p2, ja_p *p3);
//jacobian坐标系的倍点运算
//2p1=p3
extern void point_double(mpz_t p, ja_p *p1, ja_p *p3);
//设置jacobian坐标系上点的值
extern void point_set(ja_p *rp, ja_p *p);
//求-P(x,-y,z,zz)
extern void point_neg(ja_p *rp, ja_p *p);

//判断affine坐标系的点是否在曲线上
//如果在曲线上返回1，否则返回0
extern int point_is_on_curve(group *c, af_p *p);


//NAFw窗口法标量乘运算
//Q=kP
extern void scalar_mul_w(int w, mpz_t k, af_p *p, af_p *q, group *c);

//Comb法标量乘运算
//Q=kG
extern void scalar_mul_c(mpz_t k, af_p *q, enum curve_name ecname);

//由曲线名获得曲线参数
//如果找到曲线返回0，否则返回-1
extern int get_curve_parameters(enum curve_name ecname, group *c);

//计算预计算值
extern void pre_cal(enum curve_name ecname, ja_p *pre_p[][16]);

//获得存储的预计算值
//如果找到预计算值返回0，否则返回-1
extern int get_pre_cal_value(enum curve_name ecname, ja_p *pre_p[][16]);