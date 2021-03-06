#include "DefendSystem.h"
DefendSystem::DefendSystem(list<MisParameter> misParameters,list<TarParameter> tarParameters){
	list<MisParameter>::iterator misParameters_iterator = misParameters.begin();
	list<TarParameter>::iterator tarParameters_iterator = tarParameters.begin();
	for (int i=0; i<5;i++){
		if (tarParameters_iterator != tarParameters.end()){
			CVector tarLnchPos = tarParameters_iterator->tarLnchPos;
			int id = tarParameters_iterator->id;
			CVector tarfallSite = tarParameters_iterator->tarfallSite;
			double lnchTime = tarParameters_iterator->lnchTime;
			double totalFlyTime = tarParameters_iterator->totalFlyTime;
			tarParameters_iterator++;
			target[i] = new Target(id,lnchTime,totalFlyTime,tarLnchPos,tarfallSite);
			add(target[i]);
		}
	}
	for (int i=0; i<7;i++){
		if (misParameters_iterator != misParameters.end()){
			int id = misParameters_iterator->id;
			int type = misParameters_iterator->type;
			double radius = misParameters_iterator->radius;
			CVector pos = misParameters_iterator->pos;
			misParameters_iterator++;
			agent[i] = new Agent(id,pos,type,radius);
			add(agent[i]);
		}
	}
	
	for (int j = 0; j < 5; j++) {
		for (int i = 0; i < 7; i++) {
			couple(agent[i],agent[i]->outDestroy,target[j],target[j]->inDestroy);
		}
	}
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 7; j++) {
			couple(target[i],target[i]->outTarget,agent[j],agent[j]->inTarget);
		}
	}
	
}
DefendSystem::~DefendSystem()
{
}

