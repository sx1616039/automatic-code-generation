#include "Listener.h"
Listener::Listener(){
	cnt = 0;
}
Listener::~Listener(){
}
void Listener::stateChange(Atomic<IO_Type>* model, double t){
	if(model->getClassName() == "Hybrid") {
		Hybrid<IO_Type> *h = (Hybrid<IO_Type>*)model;
		ode_system<IO_Type> *a = h->getSystem();
		if (a->getClassName() == "MISSILE") {
			Missile *b = (Missile*)a;
			b->elapse = t;
		}
	}
	if (model->getClassName() == "RADAR") {
		Radar *a = (Radar*)model;
		a->elapse = t;
	}
	if (model->getClassName() == "CONTROLLER") {
		Controller *a = (Controller*)model;
		a->elapse = t;
	}
	if (model->getClassName() == "TARGET") {
		Target *a = (Target*)model;
		a->elapse = t;
	}
}
void Listener::outputEvent(Event<IO_Type> y, double t){
}
