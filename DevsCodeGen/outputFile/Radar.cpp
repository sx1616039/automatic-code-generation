#include "Radar.h"
int Radar::inTarget=0;
int Radar::outRadar=1;
int Radar::inStartRadar=2;
Radar::Radar(int id,CVector pos,int type){
	m_id = id;	
	if (type == 0) {
		m_radius = 600000;
	}
	else
	{
		m_radius = 400000;
	}
	
	m_radarPos = pos;	
	m_curTime = START_TIME;
	m_angle = 60;
	m_state = WAITING;
}

void Radar::delta_int()
{
	
}
void Radar::output_func(Bag<IO_Type>& yb)
{
	while (!outEvents.empty()) {
		ExchangeData data = outEvents.front();
		ExchangeData *t = new ExchangeData();
		t->e_req = data.e_req;
		t->e_target = data.e_target;
		t->e_src = data.e_src;
		t->e_tarInfo = data.e_tarInfo;
		t->e_destroyInfo = data.e_destroyInfo;
		t->e_lnchInfo = data.e_lnchInfo;
		t->e_radarInfo = data.e_radarInfo;
		t->e_startRadarInfo = data.e_startRadarInfo;
		IO_Type y;
		y.port = data.e_port;
		y.value = t;
		yb.insert(y);
		outEvents.pop_front();
	}
}

void Radar::delta_conf(const Bag<IO_Type>& xb)
{
	delta_int();
	delta_ext(0.0, xb);
}
void Radar::delta_ext(double e, const Bag<IO_Type>& xb)
{
	Bag<IO_Type>::const_iterator i = xb.begin();
	for (; i != xb.end(); i++){
		ModelType tar = (*i).value->e_target;
		ModelType src = (*i).value->e_src;
		if ((tar == BROADCAST || tar == RADAR) && src == CONTROLLER){
			if (m_state == WAITING) {
				int req = (*i).value->e_req;
				if (req == 0) {
					m_state = SCANNING;
				}
			}			
		}
		if ((tar == BROADCAST || tar == RADAR) && src == TARGET){
			if (m_state == SCANNING) {
				TarInfo t = (*i).value->e_tarInfo;
				scanning(t);
				CVector jwg = Pos2LLH((*i).value->e_tarInfo.t_Pos);
				int id = (*i).value->e_tarInfo.t_id;
			}
		}
	}
}
double Radar::ta()
{
	m_curTime = START_TIME + elapse;
	return TIME_STEP;
}

void Radar::gc_output(Bag<IO_Type>& g)
{
	Bag<IO_Type>::iterator i;
	for (i = g.begin(); i != g.end(); i++)
	{
		delete (*i).value;
	}
}

Radar::~Radar()
{
}

void Radar::scanning(TarInfo target){
	CVector position = target.t_Pos;
	position = Wgs2Btd(position, m_radarPos.x, m_radarPos.y, m_radarPos.z);
	if (position.y > 0 && position.Norm() < m_radius) {
		m_targets.push_back(target);
	}
	if (!m_targets.empty()) {
		ExchangeData data;
		data.e_req = 0;
		data.e_target = CONTROLLER;
		data.e_src = RADAR;
		data.e_radarInfo.r_id = m_id;
		data.e_radarInfo.r_targets = m_targets;
		data.e_port = outRadar;
		m_targets.clear();
		outEvents.push_back(data);
	}
}

