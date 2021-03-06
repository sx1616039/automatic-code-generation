#pragma once
#ifndef AGENT_H
#define AGENT_H
#include "ExchangeData.h"
#include "Vector.h"
#include "Missile.h"
#include "Radar.h"
#include "Controller.h"
#include <iostream>
using namespace std;
using namespace adevs;
class Agent : public adevs::Digraph<ExchangeData*> {
public:
	Agent(int id,CVector pos,int type,double radius);
	~Agent();
public:
	static int inTarget;
	static int outDestroy;
	Hybrid<IO_Type> *m_hybrid_m_missile;
	Missile *m_missile;
	Radar *m_radar;
	Controller *m_controller;
	
public:
	
};
#endif
