#pragma once
#ifndef RADAR_H
#define RADAR_H
#include "ExchangeData.h"
#include "Vector.h"
#include <iostream>
using namespace std;
using namespace adevs;
class Radar : public Atomic<IO_Type> {
public:
	Radar(int id,CVector pos,int type);
	void delta_int();
	void delta_ext(double e, const adevs::Bag<IO_Type>& xb);
	void delta_conf(const adevs::Bag<IO_Type>& xb);
	void output_func(adevs::Bag<IO_Type>& yb);
	double ta();
	void gc_output(adevs::Bag<IO_Type>& g);
	~Radar();
	const char* getClassName() { return "RADAR"; };
	State getState() {return m_state;}
	void setStateToEnd() {m_state = ENDED;}
private:
	State              m_state;
	list<ExchangeData> outEvents;
public:
	double        elapse;
	static int inTarget;
	static int outRadar;
	static int inStartRadar;
	int m_id;
	int m_tarId;
	CVector m_radarPos;
	double m_radius;
	double m_angle;
	list<TarInfo> m_targets;
	double m_lastTime;
	double m_curTime;
	
public:
	void scanning(TarInfo target);
	
};
#endif
