#include "Controller.h"
int Controller::inRadar=0;
int Controller::outLnchInfo=1;
int Controller::inDestroy=2;
int Controller::outStartRadar=3;
double Controller::m_threatIndex[NUM_AGENT][NUM_TARGET]={0};
bool Controller::m_lockedTarget[NUM_AGENT][NUM_TARGET]={false};
Controller::Controller(int id,CVector pos,int type,double radius){
	m_id = id;	
	m_centerPos = Wgs2Btd(LLH2Pos(pos.x, pos.y, pos.z), pos.x, pos.y, pos.z);
	m_pos = pos;
	m_radius = radius;
	m_curTime = START_TIME;
	if (m_type == 0) {//作战单元参数:0表示pac-3, 1表示thaad
		m_misAvrgVel = 1000;
	}
	if (m_type == 1) {
		m_misAvrgVel = 2000;
	}
	ExchangeData data;
	data.e_src = CONTROLLER;
	data.e_target = RADAR;
	data.e_req = 0;// 启动雷达
	data.e_port = outStartRadar;
	outEvents.push_back(data);
	m_type = type;
	m_state = WAITING;
}

void Controller::delta_int()
{
	
}
void Controller::output_func(Bag<IO_Type>& yb)
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

void Controller::delta_conf(const Bag<IO_Type>& xb)
{
	delta_int();
	delta_ext(0.0, xb);
}
void Controller::delta_ext(double e, const Bag<IO_Type>& xb)
{
	Bag<IO_Type>::const_iterator i = xb.begin();
	for (; i != xb.end(); i++){
		ModelType tar = (*i).value->e_target;
		ModelType src = (*i).value->e_src;
		if ((tar == BROADCAST || tar == CONTROLLER) && src == MISSILE){
			if (m_state == WAITING) {
				int tarId = (*i).value->e_destroyInfo.d_tarId;
				int misId = (*i).value->e_destroyInfo.d_misId;
				double time = (*i).value->e_destroyInfo.d_destroyTime;
				cout << m_curTime << "CONTROLLER" << m_id << "从missile" << misId << " 收到目标 " << tarId;
				cout << "摧毁的时间： " << time << endl;
			}
		}
		if ((tar == BROADCAST || tar == CONTROLLER) && src == RADAR){
			if (m_state == WAITING) {
				while (!(*i).value->e_radarInfo.r_targets.empty()) {
					TarInfo tar = (*i).value->e_radarInfo.r_targets.front();
					(*i).value->e_radarInfo.r_targets.pop_front();
					CVector tarPos = tar.t_Pos;
					CVector tarVel = tar.t_Vel;
					CVector fallSite = tar.t_fallPos;
					double lnchTime = tar.t_lnchTime;
					double totalTime = tar.t_totalTime;
					if (!m_lockedTarget[m_id][tar.t_id]) {
						m_threatIndex[m_id][tar.t_id] = threatEvaluate(tarVel.Norm() / 1000.0, lnchTime + totalTime - m_curTime, fallSite);

						m_tars[tar.t_id] = tar;

						int id = tar.t_id;
						int radarId = (*i).value->e_radarInfo.r_id;
						CVector jwg = Pos2LLH(tarPos);
						/*cout << m_curTime << "===========CONTROLLER" << m_id <<"从radar"<<radarId<< " 收到目标 " << id;
						cout << " 的经纬度： " << jwg.x << "  " << jwg.y << "  " << jwg.z << endl;*/
					}
				}	
				assignTarget();
			}		
		}
	}
}
double Controller::ta()
{
	m_curTime = START_TIME + elapse;
	return TIME_STEP;
}

void Controller::gc_output(Bag<IO_Type>& g)
{
	Bag<IO_Type>::iterator i;
	for (i = g.begin(); i != g.end(); i++)
	{
		delete (*i).value;
	}
}

Controller::~Controller()
{
}

void Controller::Kepler2(CVector *curPos,CVector *curVel,CVector launchPos,CVector launchVel,double flyTime){
	double V1, R1, a, x, x1, z=0, C=0, S=0, t=0, dtdx, f, g, df, dg;
	V1 = launchVel.Norm();
	R1 = launchPos.Norm();
	a = -MU / (V1*V1 - 2 * MU / R1);
	x = sqrt(MU)*flyTime / a;
	x1 = x + 1;
	while (fabs(x - x1) > 1e-9) {
		x = x1;
		z = x * x / a;
		C = (1 - cos(sqrt(z))) / z;
		S = (sqrt(z) - sin(sqrt(z))) / pow(z, 1.5);
		t = (launchPos^launchVel)*x*x*C / MU + ((1 - R1 / a)*pow(x, 3)*S + R1 * x) / sqrt(MU);
		dtdx = 2 * (launchPos^launchVel)*x*C / MU + (3 * (1 - R1 / a)*x*x*S + R1) / sqrt(MU);
		x1 = x + (flyTime - t) / dtdx;
	}
	f = 1 - pow(x, 2)*C / R1;
	g = t - pow(x, 3)*S / sqrt(MU);
	*curPos = f * launchPos + g * launchVel;
	df = sqrt(MU)*x / (launchPos.Norm()*(curPos->Norm()))*(z*S - 1);
	dg = 1 - pow(x, 2)*C / (curPos->Norm());
	*curVel = df * launchPos + dg * launchVel;
}

double Controller::threatEvaluate(double v,double leftTime,CVector fallSite){
	CVector TarFallSite[NUM_SITE];
	TarFallSite[0].x = 116.46; TarFallSite[0].y = 39.91;  TarFallSite[0].z = 100;
	TarFallSite[1].x = 116.30; TarFallSite[1].y = 39.91;  TarFallSite[1].z = 100;
	TarFallSite[2].x = 116.16; TarFallSite[2].y = 39.98;  TarFallSite[2].z = 100;
	TarFallSite[3].x = 116.39; TarFallSite[3].y = 39.91;  TarFallSite[3].z = 100;
	TarFallSite[4].x = 116.41; TarFallSite[4].y = 39.91;  TarFallSite[4].z = 100;

	const double siteValue[] = { 0.6, 0.7, 0.8, 0.9, 0.5 };// 要地价值	
	ImpSite g_impSite[NUM_SITE];
	for (int i = 0; i < NUM_SITE; i++) {
		g_impSite[i].pos = TarFallSite[i];
		g_impSite[i].value = siteValue[i];
	}
	char emergeDir[] = { "南" };
	fallSite = Wgs2Btd(fallSite, m_pos.x, m_pos.y, m_pos.z);
	int Gr;
	if (v <= 3) {
		Gr = 1;
	}
	else if (v <= 4) {
		Gr = 2;
	}
	else if (v <= 5) {
		if (strcmp(emergeDir, "西南") == 0) {
			Gr = 6;
		}
		else {
			Gr = 5;
		}
	}
	else if (v <= 6) {
		Gr = 7;
	}
	else {
		if (strcmp(emergeDir, "东南") == 0) {
			Gr = 8;
		}
		else {
			Gr = 10;
		}
	}
	double R_fallpointscatter = 0, R_killPower = 0.5*pow(Gr, 1.5) * 1000;
	if ((fallSite - m_centerPos).Norm() > R_fallpointscatter + R_killPower + m_radius) {
		return 0;
	}
	double D_harm, D_harmMax = 0, D_scaleMax = 0;
	int D_flt, n = 0;
	for (int i = 0; i < NUM_SITE; i++) {
		CVector t_impSite, t_refPos;
		t_impSite = Wgs2Btd(LLH2Pos(g_impSite[i].pos.x, g_impSite[i].pos.y, g_impSite[i].pos.z),
			m_pos.x, m_pos.y, m_pos.z);
		t_refPos = fallSite - t_impSite;
		if (t_refPos.Norm() > R_fallpointscatter + R_killPower) {
			continue;
		}
		if (D_scaleMax < g_impSite[i].value) {
			D_scaleMax = g_impSite[i].value;
			n++;
		}
		D_harm = 1 - t_refPos.Norm() / (R_fallpointscatter + R_killPower);
		if (D_harmMax < D_harm) {
			D_harmMax = D_harm;
		}
	}
	if (leftTime > 240) {
		D_flt = 1;
	}
	if (leftTime > 60 && leftTime <= 240) {
		D_flt = (int)((260 - leftTime) / 20);
	}
	else {
		D_flt = 10;
	}
	if (D_harmMax == 0) {
		return 0;
	}
	else {
		double x = pow(D_scaleMax, 1 / n)*(0.6*D_harmMax + 0.4*D_flt / 10);
		return x;
	}
	return 0;
}

double Controller::findMaxThreat(int *id,int *tarId){
double max = 0;
	for (int i = 0; i <NUM_AGENT; i++) {
		for (int j = 0; j < NUM_TARGET; j++) {
			if (m_threatIndex[i][j] > max) {
				max = m_threatIndex[i][j];
				*id = i;
				*tarId = j;
			}
		}
	}
	return max;
}

double Controller::interceptPlanning(CVector tarLnchPos,CVector tarLnchVel,double lnchTime,double totalTime){
	double        discoverDist; // 作战单元的发现距离
	double        misAvrgVel;   // 拦截弹的平均速度
	double        misMinFlyTime;// 导弹的最小飞行时间
	double        misMaxFlyTime;// 导弹的最大飞行时间
	double        misMinFlyHeiht;// 导弹的最小飞行高度
	double        misMaxFlyHeiht;// 导弹的最大飞行高度
	
	if (m_type == 0) {//作战单元参数:0表示pac-3, 1表示thaad
		discoverDist = 600000;
		misAvrgVel = 1000;
		misMinFlyTime = 3;
		misMaxFlyTime = 40;
		misMinFlyHeiht = -1;
		misMaxFlyHeiht = 30000;
	}
	if (m_type == 1) {
		discoverDist = 400000;
		misAvrgVel = 2000;
		misMinFlyTime = 20;
		misMaxFlyTime = 120;
		misMinFlyHeiht = 30000;
		misMaxFlyHeiht = -1;
	}
	
	double t, tl, tm, tr;
	double tarHeight;
	double idealExpTime, earlyTime, lateTime;
	CVector curPos, curVel;
	tr = m_curTime;
	tl = lnchTime + totalTime;
	do {
		tm = (tr + tl) / 2;
		Kepler2(&curPos, &curVel, tarLnchPos, tarLnchVel, tm - lnchTime);
		curPos = Wgs2Btd(curPos, m_pos.x, m_pos.y, m_pos.z);
		t = curPos.Norm() / misAvrgVel;
		if (m_curTime + t < tm) {
			tl = tm;
		}
		else {
			tr = tm;
		}
	} while (tl - tr > TIME_STEP*0.1);
	idealExpTime = tm;
	if (misMaxFlyHeiht > 0) {

		tr = idealExpTime;
		tl = lnchTime + totalTime;

		while (tl - tr > TIME_STEP*0.1) {
			tm = (tr + tl) / 2;
			Kepler2(&curPos, &curVel, tarLnchPos, tarLnchVel, tm - lnchTime);
			tarHeight = Pos2LLH(curPos).z;
			curPos = Wgs2Btd(curPos, m_pos.x, m_pos.y, m_pos.z);
			t = curPos.Norm() / misAvrgVel;
			if (tarHeight > misMaxFlyHeiht || t > misMaxFlyTime) {
				tr = tm;
			}
			else {
				tl = tm;
			}
		}
		earlyTime = tl;

		tr = idealExpTime;
		tl = lnchTime + totalTime;
		
		while (tl - tr > TIME_STEP*0.1) {
			tm = (tr + tl) / 2;
			Kepler2(&curPos, &curVel, tarLnchPos, tarLnchVel, tm - lnchTime);
			curPos = Wgs2Btd(curPos, m_pos.x, m_pos.y, m_pos.z);
			t = curPos.Norm() / misAvrgVel;
			if (t <  misMinFlyTime) {
				tl = tm;
			}
			else {
				tr = tm;
			}
		}
		lateTime = tr;
	}
	else {
		tr = idealExpTime;
		tl = lnchTime + totalTime;
		double tarHeight;
		while (tl - tr > TIME_STEP*0.1) {
			tm = (tr + tl) / 2;
			Kepler2(&curPos, &curVel, tarLnchPos, tarLnchVel, tm - lnchTime);
			tarHeight = Pos2LLH(curPos).z;
			curPos = Wgs2Btd(curPos, m_pos.x, m_pos.y, m_pos.z);
			t = curPos.Norm() /  misAvrgVel;
			if (t >  misMaxFlyTime) {
				tr = tm;
			}
			else {
				tl = tm;
			}
		}
		earlyTime = tl;

		tr = idealExpTime;
		tl = lnchTime + totalTime;
		
		while (tl - tr > TIME_STEP*0.1) {
			tm = (tr + tl) / 2;
			Kepler2(&curPos, &curVel, tarLnchPos, tarLnchVel, tm - lnchTime);
			curPos = Wgs2Btd(curPos, m_pos.x, m_pos.y, m_pos.z);
			t = curPos.Norm() /  misAvrgVel;
			if (tarHeight <  misMinFlyHeiht || t <  misMinFlyTime) {
				tl = tm;
			}
			else {
				tr = tm;
			}
		}
		lateTime = tr;
	}
	
	if (earlyTime >= lateTime) {
		return 0;
	}
	return earlyTime;	
}

void Controller::assignTarget(){
	int conId = -1;
	int tarId = -1;
	double maxThreat = findMaxThreat(&conId, &tarId);
	
	if (conId == m_id) {
		double meetTime = interceptPlanning(m_tars[tarId].t_lnchPos, m_tars[tarId].t_lnchVel, m_tars[tarId].t_lnchTime, m_tars[tarId].t_totalTime);
		cout << m_curTime << "===================================最大威胁" << maxThreat << "===CONTROLLER：" << conId << "===目标：" << tarId << endl;
		if (meetTime > 0.1) {
			CVector curPos, curVel;
			Kepler2(&curPos, &curVel, m_tars[tarId].t_lnchPos, m_tars[tarId].t_lnchVel, meetTime - m_tars[tarId].t_lnchTime);
			curPos = Wgs2Btd(curPos, m_pos.x, m_pos.y, m_pos.z);
			double t = curPos.Norm() / m_misAvrgVel;

			LnchInfo misLnchInfo;
			misLnchInfo.l_lnchTime = meetTime - t;
			misLnchInfo.l_curTime = m_curTime;
			misLnchInfo.l_lnchPos = m_pos;
			misLnchInfo.l_lnchVel = curPos.Unit() * 2000;
			misLnchInfo.l_tarPos = m_tars[tarId].t_Pos;
			misLnchInfo.l_tarVel = m_tars[tarId].t_Vel;
			misLnchInfo.l_tarId = tarId;
			misLnchInfo.l_misId = m_id;
			
			ExchangeData data;
			data.e_src = CONTROLLER;
			data.e_target = MISSILE;
			data.e_lnchInfo = misLnchInfo;
			data.e_port = outLnchInfo;
			outEvents.push_back(data);
			m_lockedTarget[m_id][tarId] = true;
			for (int i = 0; i < NUM_TARGET; i++) {
				m_threatIndex[m_id][i] = 0;
				m_lockedTarget[m_id][i] = true;
			}
			for (int i = 0; i < NUM_AGENT; i++) {
				m_threatIndex[i][tarId] = 0;
				m_lockedTarget[i][tarId] = true;
			}
		}			
	}
}

