#pragma once
#ifndef LISTENER_H
#define LISTENER_H
#include "ExchangeData.h"
#include "Missile.h"
#include "Radar.h"
#include "Controller.h"
#include "Target.h"
class Listener: public EventListener<IO_Type>{
public:
	Listener();
	~Listener();
	void outputEvent(Event<IO_Type> y, double t);
	void stateChange(Atomic<IO_Type>* model, double t);
public:
	int cnt;
};
#endif
