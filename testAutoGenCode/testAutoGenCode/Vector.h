// Vector.h: interface for the CVector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VECTOR_H__C5008964_2824_4D92_878C_B07C5856C020__INCLUDED_)
#define AFX_VECTOR_H__C5008964_2824_4D92_878C_B07C5856C020__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "math.h"

class CVector  
{
public:
	CVector Zero(void);
	CVector Unit();
	CVector Cross(CVector C1);
	double Dot(CVector C1);
	double Norm();
	CVector operator +(CVector C2);
	CVector operator -(CVector C2);
	CVector operator * (double num);
	CVector operator / (double num);
	double operator ^ (CVector C2);
	friend CVector operator * (double num, CVector C2);				//	向量乘法运算符的重载
		
	double x;
	double y;
	double z;

	CVector();
	virtual ~CVector();

};
CVector LLH2Pos(double longitude, double B, double height);
CVector Pos2LLH(CVector pos);
CVector Btd2Wgs(CVector C1, double longitude, double B, double height);
CVector Wgs2Btd(CVector C1, double longitude, double B, double height);
CVector Btd2Wgs_Vel(CVector C1, double longitude, double B);
CVector Wgs2Btd_Vel(CVector C1, double longitude, double B);
double KillRate(bool m_type, double H, CVector t_vel, CVector m_vel);
#endif // !defined(AFX_VECTOR_H__C5008964_2824_4D92_878C_B07C5856C020__INCLUDED_)
