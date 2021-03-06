#include "Missile.h"
int Missile::inLnchInfo=0;
int Missile::outDestroy=1;
int Missile::inTarget=2;
Missile::Missile(int id,int type):ode_system<IO_Type>(6,0 ){
		this->m_id = id;	
	if (type == 0) {// PC-3
		m_maxFlyTime = 40;		
	}
	else if (type == 1) {// THAAD
		m_maxFlyTime = 120;		
	}

	m_state = WAITING;	
	m_lnchTime = DBL_MAX;
	m_lnchPos = m_lnchPos.Zero();
	m_curPos = m_curPos.Zero();
	m_curVel = m_curVel.Zero();
	m_tarId = -1;
	m_curTime = START_TIME;
	m_lastDist = 5000000;

	char a[10];
	sprintf_s(a, "%d", m_id);
	string fileName = "Missile" + string(a) + ".csv";
	m_outFile.open(fileName);
	if (!m_outFile) {
		cout << "cannot open file " << fileName << endl;
	}
}

void Missile::init(double* q) {
	for (int i = 0; i < 6; i++) {
		q[i] = 0;
	}
	q[6] = m_curTime;
}
void Missile::der_func(const double* q, double* dq) {
	CVector der_m_curPos;
	m_curPos.x = q[0];
	m_curPos.y = q[1];
	m_curPos.z = q[2];
	CVector der_m_curVel;
	m_curVel.x = q[3];
	m_curVel.y = q[4];
	m_curVel.z = q[5];
	m_acc = calculateAcc(m_curPos, m_curVel);
der_m_curPos=m_curVel;
der_m_curVel=m_acc;

	dq[0] = der_m_curPos.x;
	dq[1] = der_m_curPos.y;
	dq[2] = der_m_curPos.z;
	dq[3] = der_m_curVel.x;
	dq[4] = der_m_curVel.y;
	dq[5] = der_m_curVel.z;
	dq[6] = 1;
}
double Missile::time_event_func(const double* q)
{
	m_curTime = START_TIME + elapse;
	if (m_state == HITTED){
			m_state = ENDED;
	return 0.000001;
	}
	if (m_state == FLYING && (m_curPos - m_tarPos).Norm() < HIT_ERROR){
				m_state = HITTED;
		ExchangeData data;
		data.e_src = MISSILE;
		data.e_target = BROADCAST;// 广播
		data.e_destroyInfo.d_tarId = m_tarId;
		data.e_destroyInfo.d_misId = m_id;
		data.e_destroyInfo.d_destroyTime = m_curTime;
		data.e_port = outDestroy;
		outEvents.push_back(data);
	m_state = HITTED;
	}
	if (m_state == FLYING &&  m_curTime - m_lnchTime > m_maxFlyTime){
		m_state = LANDED;
	}
	if (m_state == PREPARING && m_curTime >= m_lnchTime){
		m_state = FLYING;
	}
	return DBL_MAX;
}
void Missile::internal_event(double* q, const bool* state_event) {
}

void Missile::output_func(const double *q, const bool* state_event, Bag<IO_Type>& yb)
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

void Missile::state_event_func(const double* q, double *z) {
}
void Missile::external_event(double* q, double e, const Bag<IO_Type>& xb) {
	Bag<IO_Type>::const_iterator i = xb.begin();
	for (; i != xb.end(); i++){
		ModelType tar = (*i).value->e_target;
		ModelType src = (*i).value->e_src;
		if ((tar == BROADCAST || tar == MISSILE) && src == CONTROLLER){
			if (m_state == WAITING) {
				int id = (*i).value->e_lnchInfo.l_misId;
				if (id == m_id) {
					m_lnchTime = (*i).value->e_lnchInfo.l_lnchTime;
					m_tarId = (*i).value->e_lnchInfo.l_tarId;
					m_lnchPos = (*i).value->e_lnchInfo.l_lnchPos;
					m_curVel = (*i).value->e_lnchInfo.l_lnchVel;
					m_tarPos = (*i).value->e_lnchInfo.l_tarPos;
					m_tarVel = (*i).value->e_lnchInfo.l_tarVel;
					m_curTime = (*i).value->e_lnchInfo.l_curTime;
					m_state = PREPARING;

					m_tarPos = Wgs2Btd(m_tarPos, m_lnchPos.x, m_lnchPos.y, m_lnchPos.z);
					m_tarVel = Wgs2Btd_Vel(m_tarVel, m_lnchPos.x, m_lnchPos.y);
					q[3] = m_curVel.x;
					q[4] = m_curVel.y;
					q[5] = m_curVel.z;
					q[0] = 0;
					q[1] = 0;
					q[2] = 0;
					q[6] = m_curTime;		
					cout << q[6] << "=====Missile  " << m_id << "  打击目标  " << m_tarId << " 发射时间：" << m_lnchTime << "  ";
					cout << m_tarVel.x << " " << m_tarVel.y << "   " << m_tarVel.y << endl;
				}
			}
		}
		if ((tar == BROADCAST || tar == MISSILE) && src == TARGET){
			if (m_state == FLYING) {
				int id = (*i).value->e_tarInfo.t_id;
				if (id == m_tarId) {
					m_tarPos = (*i).value->e_tarInfo.t_Pos;
					m_tarVel = (*i).value->e_tarInfo.t_Vel;
					m_tarPos = Wgs2Btd(m_tarPos, m_lnchPos.x, m_lnchPos.y, m_lnchPos.z);
					m_tarVel = Wgs2Btd_Vel(m_tarVel, m_lnchPos.x, m_lnchPos.y);

					CVector jwg = Pos2LLH((*i).value->e_tarInfo.t_Pos);
					/*cout << q[T] << "Missile" << m_id << " 收到目标" << id;
					cout << " 的经纬度： " << jwg.x << "  " << jwg.y << "  " << jwg.z << endl;*/
					m_curPos.x = q[0];
					m_curPos.y = q[1];
					m_curPos.z = q[2];
					CVector pos = Btd2Wgs(m_curPos, m_lnchPos.x, m_lnchPos.y, m_lnchPos.z);
					CVector Mjwg = Pos2LLH(pos);
					m_outFile << q[6] << "," << m_id << "," << Mjwg.x << "," << Mjwg.y << "," << Mjwg.z;
					m_outFile << "," << m_tarId << "," << jwg.x << "," << jwg.y << "," << jwg.z;
					m_outFile << "," << m_tarId << "," << m_tarVel.x << "," << m_tarVel.y << "," << m_tarVel.z << endl;
				}
			}
		}
	}
}
void Missile::confluent_event(double* q, const bool* state_event,const Bag<IO_Type>& xb) {
	internal_event(q, state_event);
	external_event(q, 0.0, xb);
}

void Missile::postStep(double* q) {
}

void Missile::gc_output(Bag<IO_Type>& g)
{
	Bag<IO_Type>::iterator i;
	for (i = g.begin(); i != g.end(); i++)
	{
		delete (*i).value;
	}
}

Missile::~Missile()
{
	m_outFile.close();
}

CVector Missile::calculateAcc(CVector curPosition,CVector curVel){
		double t_go;
	int M_K = 2;
	CVector relPos, relVel, radialVel, tangentVel, zem, missileAcc;
	relPos = m_tarPos - curPosition;
	relVel = curVel - m_tarVel;
	radialVel = relPos.Unit()*relVel.Norm()*relPos.Unit().Dot(relVel.Unit());
	tangentVel = relVel - radialVel;
	t_go = relPos.Norm() / radialVel.Norm();
	zem = tangentVel * t_go;
	missileAcc = zem * (-1)*M_K / t_go / t_go;
	return missileAcc;
}

	