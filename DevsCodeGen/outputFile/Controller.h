#pragma once
#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "ExchangeData.h"
#include "Vector.h"
#include <iostream>
using namespace std;
using namespace adevs;
class Controller : public Atomic<IO_Type> {
public:
	Controller(int id,CVector pos,int type,double radius);
	void delta_int();
	void delta_ext(double e, const adevs::Bag<IO_Type>& xb);
	void delta_conf(const adevs::Bag<IO_Type>& xb);
	void output_func(adevs::Bag<IO_Type>& yb);
	double ta();
	void gc_output(adevs::Bag<IO_Type>& g);
	~Controller();
	const char* getClassName() { return "CONTROLLER"; };
	State getState() {return m_state;}
	void setStateToEnd() {m_state = ENDED;}
private:
	State              m_state;
	list<ExchangeData> outEvents;
public:
	double        elapse;
	static int inRadar;
	static int outLnchInfo;
	static int inDestroy;
	static int outStartRadar;
	double m_lastTime;
	static double m_threatIndex[NUM_AGENT][NUM_TARGET];
	static bool m_lockedTarget[NUM_AGENT][NUM_TARGET];
	int m_id;
	CVector m_pos;
	CVector m_centerPos;
	double m_curTime;
	double m_radius;
	int m_type;
	TarInfo m_tars[5];
	double m_misAvrgVel;
	
public:
	void Kepler2(CVector *curPos,CVector *curVel,CVector launchPos,CVector launchVel,double flyTime);
	double threatEvaluate(double v,double leftTime,CVector fallSite);
	double findMaxThreat(int *id,int *tarId);
	double interceptPlanning(CVector tarLnchPos,CVector tarLnchVel,double lnchTime,double totalTime);
	void assignTarget();
	
};
#endif
