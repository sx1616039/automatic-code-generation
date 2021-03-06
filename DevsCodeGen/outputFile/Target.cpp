#include "Target.h"
int Target::inDestroy=0;
int Target::outTarget=1;
Target::Target(int id,double lnchTime,double totalFlyTime,CVector tarLnchPos,CVector tarfallSite){
	m_id = id;
	m_lnchTime = lnchTime;
	m_totalFlyTime = totalFlyTime;
	m_lnchPos = LLH2Pos(tarLnchPos.x, tarLnchPos.y, tarLnchPos.z);
	
	m_fallSite = LLH2Pos(tarfallSite.x, tarfallSite.y, tarfallSite.z);
	m_lnchVel = Gauss(m_lnchPos, m_fallSite, totalFlyTime);
	m_curVel = m_lnchVel;
	m_curPos = m_lnchPos;
	m_state = WAITING;	
	m_curTime = START_TIME;
}

void Target::delta_int()
{
	if (m_state == FLYING){
			Kepler3(&m_curPos, &m_curVel, m_lnchPos,m_lnchVel, m_curTime - m_lnchTime);
		ExchangeData data;
		data.e_src = TARGET;
		data.e_target = BROADCAST;// 广播
		data.e_tarInfo.t_id = m_id;
		data.e_tarInfo.t_lnchTime = m_lnchTime;
		data.e_tarInfo.t_Pos = m_curPos;
		data.e_tarInfo.t_Vel = m_curVel;
		data.e_tarInfo.t_lnchVel = m_lnchVel;
		data.e_tarInfo.t_lnchPos = m_lnchPos;
		data.e_tarInfo.t_fallPos = m_fallSite;
		data.e_tarInfo.t_totalTime = m_totalFlyTime;
		data.e_port = outTarget;
		outEvents.push_back(data);
	}
	if (m_state == WAITING &&  m_curTime >= m_lnchTime){
		m_state = FLYING;
	}
	if (m_state == FLYING && m_totalFlyTime + m_lnchTime <= m_curTime){
		m_state = LANDED;
	}
	
}
void Target::output_func(Bag<IO_Type>& yb)
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

void Target::delta_conf(const Bag<IO_Type>& xb)
{
	delta_int();
	delta_ext(0.0, xb);
}
void Target::delta_ext(double e, const Bag<IO_Type>& xb)
{
	Bag<IO_Type>::const_iterator i = xb.begin();
	for (; i != xb.end(); i++){
		ModelType tar = (*i).value->e_target;
		ModelType src = (*i).value->e_src;
		if ((tar == BROADCAST || tar == TARGET) && src == MISSILE){
			int id = (*i).value->e_destroyInfo.d_tarId;
			if (id == m_id) {
				m_state = DESTROYED;
				cout << "target------------DESTROYED------------------------" << m_id << endl;
			}
		}
	}
}
double Target::ta()
{
	m_curTime = START_TIME + elapse;
	return TIME_STEP;
}

void Target::gc_output(Bag<IO_Type>& g)
{
	Bag<IO_Type>::iterator i;
	for (i = g.begin(); i != g.end(); i++)
	{
		delete (*i).value;
	}
}

Target::~Target()
{
}

void Target::Kepler3(CVector *curPos,CVector *curVel,CVector lnchPos,CVector lnchVel,double e){
	const double MU = 3.986005e14;
	double V1, R1, a, x, x1, z = 0, C = 0, S = 0, t = 0, dtdx, f, g, df, dg;
	V1 = lnchVel.Norm();
	R1 = lnchPos.Norm();
	a = -MU / (V1 * V1 - 2 * MU / R1);
	x = sqrt(MU) * e / a;
	x1 = x + 1;
	while (fabs(x - x1) > 1e-9) {
		x = x1;
		z = x * x / a;
		C = (1 - cos(sqrt(z))) / z;
		S = (sqrt(z) - sin(sqrt(z))) / pow(z, 1.5);
		t = (lnchPos ^ lnchVel) * x * x * C / MU + ((1 - R1 / a) * pow(x, 3) * S + R1 * x) / sqrt(MU);
		dtdx = 2 * (lnchPos ^ lnchVel) * x * C / MU + (3 * (1 - R1 / a) * x * x * S + R1) / sqrt(MU);
		x1 = x + (e - t) / dtdx;
	}
	f = 1 - pow(x, 2) * C / R1;
	g = t - pow(x, 3) * S / sqrt(MU);
	*curPos = f * lnchPos + g * lnchVel;
	df = sqrt(MU) * x / (lnchPos.Norm() * (curPos->Norm())) * (z * S - 1);
	dg = 1 - pow(x, 2) * C / (curPos->Norm());
	*curVel = df * lnchPos + dg * lnchVel;
}

CVector Target::Gauss(CVector launchPos,CVector fallSite,double totalFlyTime){
double n = 0;
	double R1, R2, deltaf, A, z1, z2, z, C, S, y, x, t, f, g, dgdt;
	CVector v1;
	R1 = launchPos.Norm();
	R2 = fallSite.Norm();
	deltaf = acos((launchPos^fallSite) / (R1 * R2));
	A = sqrt(R1*R2)*sin(deltaf) / sqrt(1 - cos(deltaf));
	z1 = 0;
	z2 = 4 * PI*PI;
	while (fabs(z2 - z1) > 1e-7) {
		n++;
		z = (z1 + z2) / 2;
		C = (1 - cos(sqrt(z))) / z;
		S = (sqrt(z) - sin(sqrt(z))) / pow(z, 1.5);
		y = R1 + R2 - A * (1 - z * S) / sqrt(C);//1 - cos(deltaf)
		x = sqrt(y / C);
		t = (pow(x, 3)*S + A * sqrt(y)) / sqrt(MU);
		if (t < totalFlyTime) {
			z1 = z;
		}
		else {
			z2 = z;
		}
	}
	f = 1 - y / R1;
	g = t - pow(x, 3)*S / sqrt(MU);
	dgdt = 1 - y / R2;
	v1 = 1 / g * (fallSite - f * launchPos);
	return v1;
}

