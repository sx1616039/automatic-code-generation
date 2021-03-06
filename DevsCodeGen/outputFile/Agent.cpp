#include "Agent.h"
int Agent::inTarget=0;
int Agent::outDestroy=1;
Agent::Agent(int id,CVector pos,int type,double radius){
	m_missile = new Missile(id,type);
	ode_solver<IO_Type>* ode_solver;
	ode_solver = new corrected_euler<IO_Type>(m_missile, 1E-1, 0.02);
	event_locator<IO_Type>* event_find = new bisection_event_locator<IO_Type>(m_missile, 500);
	m_hybrid_m_missile= new Hybrid<IO_Type>(m_missile, ode_solver, event_find);
	add(m_hybrid_m_missile);
	m_radar = new Radar(id,pos,type);
	add(m_radar);
	m_controller = new Controller(id,pos,type,radius);
	add(m_controller);
	
	couple(this,this->inTarget,m_radar,m_radar->inTarget);
	couple(m_hybrid_m_missile,m_missile->outDestroy,this,this->outDestroy);
	couple(m_radar,m_radar->outRadar,m_controller,m_controller->inRadar);
	couple(m_controller,m_controller->outLnchInfo,m_hybrid_m_missile,m_missile->inLnchInfo);
	couple(m_hybrid_m_missile,m_missile->outDestroy,m_controller,m_controller->inDestroy);
	couple(this,this->inTarget,m_hybrid_m_missile,m_missile->inTarget);
	couple(m_controller,m_controller->outStartRadar,m_radar,m_radar->inStartRadar);
	
}
Agent::~Agent()
{
}

