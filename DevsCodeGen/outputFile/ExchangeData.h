#pragma once
#ifndef EXCHANGEDATA_H
#define EXCHANGEDATA_H
#include "adevs\include\adevs.h"
#include "Vector.h"
#include "fstream"
const double TIME_STEP = 0.2;
const double START_TIME = 860.0;
const int NUM_TARGET = 5;
const int NUM_AGENT = 7;
const int NUM_MISSILE = 7;
const int NUM_CONTROLLER = 7;
const int NUM_RADAR = 7;
const int NUM_SITE = 5;
const double MU = 3.986005E14;
const int DIM = 3;
const int NUM_EVENTS = 1;
const double HIT_ERROR = 500.0;
const double M_K = 2.0;
const double PI = 3.1415926536;
enum State{ LANDED,FLYING,PROCESSING,WAITING,HITTED,DESTROYED,PREPARING,SCANNING,ENDED };
enum ModelType{ DEFENDSYSTEM,AGENT,MISSILE,RADAR,CONTROLLER,LNCHINFO,RADARINFO,TARGET,BROADCAST };
typedef struct TarInfo {
	int t_id;
	double t_lnchTime;
	CVector t_Pos;
	double t_totalTime;
	CVector t_Vel;
	CVector t_lnchVel;
	CVector t_lnchPos;
	CVector t_fallPos;
	
}TarInfo;
typedef struct LnchInfo {
	double l_lnchTime;
	int l_misId;
	int l_tarId;
	CVector l_lnchPos;
	CVector l_lnchVel;
	CVector l_tarPos;
	CVector l_tarVel;
	double l_curTime;
	
}LnchInfo;
typedef struct StartRadarInfo {
	bool isStart;
	
}StartRadarInfo;
typedef struct RadarInfo {
	int r_id;
	list<TarInfo> r_targets;
	
}RadarInfo;
typedef struct DestroyInfo {
	int d_tarId;
	int d_misId;
	double d_destroyTime;
	
}DestroyInfo;
typedef struct ImpSite {
	CVector pos;
	double value;
	
}ImpSite;
typedef struct TarParameter {
	CVector tarLnchPos;
	int id;
	CVector tarfallSite;
	double lnchTime;
	double totalFlyTime;
	
}TarParameter;
typedef struct MisParameter {
	int id;
	int type;
	double radius;
	CVector pos;
	
}MisParameter;
struct ExchangeData{
	int           e_req;
	int           e_port;
	ModelType     e_target;
	ModelType     e_src;
	TarInfo e_tarInfo;
	DestroyInfo e_destroyInfo;
	LnchInfo e_lnchInfo;
	RadarInfo e_radarInfo;
	StartRadarInfo e_startRadarInfo;
	
 };
typedef adevs::PortValue<ExchangeData*> IO_Type;
#endif
