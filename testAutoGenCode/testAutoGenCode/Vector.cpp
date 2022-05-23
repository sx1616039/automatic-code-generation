// Vector.cpp: implementation of the CVector class.
//
//////////////////////////////////////////////////////////////////////

#include "Vector.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#define PI 3.1415926
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVector::CVector()
{

}

CVector::~CVector()
{

}

CVector CVector::operator + (CVector C2)
{
	CVector C;
	C.x=C2.x+x;
	C.y=C2.y+y;
	C.z=C2.z+z;
	return C;
}

CVector CVector::operator - (CVector C2)
{
	CVector C;
	C.x=x-C2.x;
	C.y=y-C2.y;
	C.z=z-C2.z;
	return C;
}

CVector CVector::operator * (double num)
{
	CVector C;
	C.x=x*num;
	C.y=y*num;
	C.z=z*num;
	return C;
}
double CVector::operator ^ (CVector C2)
{
	return x*C2.x+y*C2.y+z*C2.z;
}

CVector operator * (double num, CVector C2)					//	向量乘法运算符的重载
{
	return C2*num;
	
}
CVector CVector::operator / (double num)
{
	CVector C;
	C.x=x/num;
	C.y=y/num;
	C.z=z/num;
	return C;
}

double CVector::Norm()
{
	return sqrt(x*x+y*y+z*z);
}

double CVector::Dot(CVector C1)
{
	double d=0.0;
	d=x*C1.x+y*C1.y+z*C1.z;
	return d;
}

CVector CVector::Cross(CVector C1)
{
	CVector C;
	C.x=y*C1.z-C1.y*z;
	C.y=-(x*C1.z-C1.x*z);
	C.z=x*C1.y-C1.x*y;
	return C;
}

CVector CVector::Unit()
{
	if (Norm()>0.0) 
	{
		return *this/Norm();

	}
	else
	{
		return *this;
	}
}

CVector CVector::Zero(void)
{
	CVector C;
	C.x = 0;
	C.y = 0;
	C.z = 0;
	return C;
}

//////////////////////////////////////////////////////////////////////
// Most Important Function as Follow
//////////////////////////////////////////////////////////////////////
CVector LLH2Pos(double longitude,double B,double height)
{
	//longitude代表经度
	//B代表纬度
	//height代表高度
	
	CVector pos;
	double a=6378137;		//赤道半径equatorial radius
	double f,e,N;
	double l,b;
	l=longitude*PI/180.0;
	b=B*PI/180.0;
	
	f=1/298.257223563;		//扁率
	e=sqrt(2*f-f*f);		//偏心率
	N=a/sqrt(1-e*e*sin(b)*sin(b));		//该点的卯酉曲率半径
	
	pos.x=(N+height)*cos(b)*cos(l);
	pos.y=(N+height)*cos(b)*sin(l);
	pos.z=(N*(1-e*e)+height)*sin(b);
	
	return pos;
}

CVector Pos2LLH(CVector pos)
{
	//longitude代表经度
	//B代表纬度
	//height代表高度
	
	CVector LLH;
	double a=6378137,f,e,phi;
	f=1/298.257223563;
	e=sqrt(2*f-f*f);
	
	LLH.x=atan2(pos.y,pos.x)*180.0/PI;					//经度
	
	phi=atan(pos.z/sqrt(pos.x*pos.x+pos.y*pos.y));		//地心纬度
	LLH.y=atan(tan(phi)/((1-f)*(1-f)))*180.0/PI;		//地理纬度
	LLH.z=pos.Norm()-a*(1-f*sin(phi)*sin(phi)-0.5*f*f*sin(2*phi)*(a/pos.Norm()-0.25));										//高度
	
	return LLH;
}

CVector Btd2Wgs(CVector C1,double longitude,double B,double height)
{
	CVector C2,C3;	//中间变量，返回值
	CVector btdZero;	//BTD坐标系坐标原点在WGS84系的坐标
	double l,b;
	l=longitude*PI/180.0;
	b=B*PI/180.0;
	btdZero=LLH2Pos(longitude,B,height);	//输入的就是地理经纬度
	//先坐标变换后平移
	//坐标变换（btd到wgs84）
	C2.x=-sin(b)*cos(l)*C1.x+cos(b)*cos(l)*C1.y-sin(l)*C1.z;
	C2.y=-sin(b)*sin(l)*C1.x+cos(b)*sin(l)*C1.y+cos(l)*C1.z;
	C2.z=cos(b)*C1.x+sin(b)*C1.y;
	//平移
	C3=C2+btdZero;
	return C3;
}

CVector Wgs2Btd(CVector C1,double longitude,double B,double height)
{
	CVector C2,C3;	//中间变量，返回值
	CVector btdZero;	//BTD坐标系坐标原点在WGS84系的坐标
	double l,b;
	l=longitude*PI/180.0;
	b=B*PI/180.0;
	btdZero=LLH2Pos(longitude,B,height);	//输入的就是地理经纬度
	//先平移后坐标变换
	//平移
	C2=C1-btdZero;
	//坐标变换（wgs84到btd）
	C3.x=-sin(b)*cos(l)*C2.x-sin(b)*sin(l)*C2.y+cos(b)*C2.z;
	C3.y=cos(b)*cos(l)*C2.x+cos(b)*sin(l)*C2.y+sin(b)*C2.z;
	C3.z=-sin(l)*C2.x+cos(l)*C2.y;
//	double pp;
//	pp=C3.Norm();
	return C3;
}

CVector Btd2Wgs_Vel(CVector C1,double longitude,double B)
{
	//B为地理纬度
	double l,b;
	l=longitude*PI/180.0;
	b=B*PI/180.0;
	CVector C2;	//返回值
	//坐标变换=速度变换（btd到wgs84）
	C2.x=-sin(b)*cos(l)*C1.x+cos(b)*cos(l)*C1.y-sin(l)*C1.z;
	C2.y=-sin(b)*sin(l)*C1.x+cos(b)*sin(l)*C1.y+cos(l)*C1.z;
	C2.z=cos(b)*C1.x+sin(b)*C1.y;
	return C2;
}

CVector Wgs2Btd_Vel(CVector C1,double longitude,double B)
{
	//B为地理纬度
	double l,b;
	l=longitude*PI/180.0;
	b=B*PI/180.0;
	CVector C2;	//返回值
	//坐标变换=速度变换（wgs84到btd）
	C2.x=-sin(b)*cos(l)*C1.x-sin(b)*sin(l)*C1.y+cos(b)*C1.z;
	C2.y=cos(b)*cos(l)*C1.x+cos(b)*sin(l)*C1.y+sin(b)*C1.z;
	C2.z=-sin(l)*C1.x+cos(l)*C1.y;
	return C2;
}

double KillRate(bool m_type, double H, CVector t_vel, CVector m_vel)
{
	double p_h, p_v, v;
	v = (t_vel - m_vel).Norm();
	if (m_type == 1)
	{
		if (H <= 45) p_h = 0.5;
			else if ( (H > 45) && (H <= 55) ) p_h = 0.6;
					else if ( (H > 55) && (H <= 65) )p_h = 0.7;
							else if ( (H > 65) && (H <= 75) ) p_h = 0.75;
									else if ( (H > 75) && (H <= 85) ) p_h = 0.8;
											else if ( (H > 85) && (H <= 95) ) p_h = 0.85;
													else  p_h = 0.9;
		if (v <= 3) p_v = 0.9;
			else if ( (v > 3) && (v <= 5) ) p_v = 0.8;
					else if ( (v > 5) && (v <= 7) ) p_v = 0.7;
							else p_v = 0.5;
	}
	else
	{
		if (H <= 2) p_h = 0.6;
			else if ( (H > 2) && (H <= 4) ) p_h = 0.7;
					else if ( (H > 4) && (H <= 7.5) ) p_h = 0.8;
							else if ( (H > 7.5) && (H <= 12.5) ) p_h = 0.9;
									else if ( (H > 12.5) && (H <= 17.5) ) p_h = 0.9;
											else if ( (H > 17.5) && (H <= 22.5) ) p_h = 0.8;
													else  if ( (H > 22.5) && (H <= 26) ) p_h = 0.7;
															else if ( (H > 26) && (H <= 27) ) p_h = 0.5;
																	else p_h = 0;
		if (v <= 2.5) p_v = 0.9;
			else if ( (v > 2.5) && (v <= 3.5) ) p_v = 0.85;
					else if ( (v > 3.5) && (v <= 4.5) ) p_v = 0.8;
							else if ( (v > 4.5) && (v <= 5.5) ) p_v = 0.75;
									else  p_v = 0.7;
	}
	return p_h * p_v;
}
