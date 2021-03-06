#pragma once
#ifndef DEFENDSYSTEM_H
#define DEFENDSYSTEM_H
#include "ExchangeData.h"
#include "Vector.h"
#include "Target.h"
#include "Agent.h"
#include <iostream>
using namespace std;
using namespace adevs;
class DefendSystem : public adevs::Digraph<ExchangeData*> {
public:
	DefendSystem(list<MisParameter> misParameters,list<TarParameter> tarParameters);
	~DefendSystem();
public:
	Target *target[5];
	Agent *agent[7];
	
public:
	
};
#endif
