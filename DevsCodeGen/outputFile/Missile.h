#pragma once
#ifndef MISSILE_H
#define MISSILE_H
#include "ExchangeData.h"
#include "Vector.h"
#include <iostream>
using namespace std;
using namespace adevs;
class Missile : public ode_system<IO_Type> {
public:
	Missile(int id,int type);
	void init(double *q);
	void der_func(const double* q, double* dq);
	void state_event_func(const double* q, double *z);
	double time_event_func(const double* q);
	void external_event(double* q, double e, const Bag<IO_Type>& xb);
	void internal_event(double* q, const bool* state_event);
	void confluent_event(double* q, const bool* state_event, const Bag<IO_Type>& xb);
	void output_func(const double *q, const bool* state_event, Bag<IO_Type>& yb);
	void postStep(double* q);
	void gc_output(Bag<IO_Type>&);
	State getState() {return m_state; };
	~Missile();
	const char* getClassName() { return "MISSILE"; };
private:
	State              m_state;
	ofstream           m_outFile;
	list<ExchangeData> outEvents;
public:
	double        elapse;
	static int inLnchInfo;
	static int outDestroy;
	static int inTarget;
	CVector m_curPos;
	CVector m_curVel;
	int m_id;
	double m_lnchTime;
	double m_maxFlyTime;
	CVector m_tarPos;
	int m_tarId;
	CVector m_tarVel;
	CVector m_lnchPos;
	double m_lastDist;
	double m_curTime;
	CVector m_acc;
	
public:
	CVector calculateAcc(CVector curPosition,CVector curVel);
};
#endif
