#pragma once
#ifndef TARGET_H
#define TARGET_H
#include "ExchangeData.h"
#include "Vector.h"
#include <iostream>
using namespace std;
using namespace adevs;
class Target : public Atomic<IO_Type> {
public:
	Target(int id,double lnchTime,double totalFlyTime,CVector tarLnchPos,CVector tarfallSite);
	void delta_int();
	void delta_ext(double e, const adevs::Bag<IO_Type>& xb);
	void delta_conf(const adevs::Bag<IO_Type>& xb);
	void output_func(adevs::Bag<IO_Type>& yb);
	double ta();
	void gc_output(adevs::Bag<IO_Type>& g);
	~Target();
	const char* getClassName() { return "TARGET"; };
	State getState() {return m_state;}
	void setStateToEnd() {m_state = ENDED;}
private:
	State              m_state;
	list<ExchangeData> outEvents;
public:
	double        elapse;
	static int inDestroy;
	static int outTarget;
	int m_id;
	double m_totalFlyTime;
	double m_lnchTime;
	CVector m_curPos;
	CVector m_curVel;
	CVector m_lnchPos;
	CVector m_fallSite;
	CVector m_lnchVel;
	double m_curTime;
	
public:
	void Kepler3(CVector *curPos,CVector *curVel,CVector lnchPos,CVector lnchVel,double e);
	CVector Gauss(CVector launchPos,CVector fallSite,double totalFlyTime);
	
};
#endif
