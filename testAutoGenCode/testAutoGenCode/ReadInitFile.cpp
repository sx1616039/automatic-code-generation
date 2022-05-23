#include "ReadInitFile.h"



ReadInitFile::ReadInitFile(char fileName[])
{
	m_doc = new TiXmlDocument(fileName);
	bool loadOkay = m_doc->LoadFile();
	if (!loadOkay)
	{
		printf("Could not load test file 'demotest.xml'. Error='%s'. Exiting.\n", m_doc->ErrorDesc());
		exit(0);
	}
	readFile();
}
void ReadInitFile::readFile() {
	TiXmlElement *inRoot = m_doc->FirstChildElement(); //root指向xml文档的第一个节
	if (inRoot != NULL) {
		for (TiXmlElement *inElem = inRoot->FirstChildElement(); inElem != NULL; inElem = inElem->NextSiblingElement()) {
			if (strcmp(inElem->Attribute("camp"), "red") == 0) {
				int id = -1, type;
				double radius;
				for (TiXmlElement *var = inElem->FirstChildElement(); var != NULL; var = var->NextSiblingElement()) {
					if (strcmp(var->Attribute("name"), "m_id") == 0) {
						id = atoi(var->Attribute("value"));
					}
					else if (strcmp(var->Attribute("name"), "m_type") == 0) {
						string stype = var->Attribute("value");
						if (stype == "PC-3") {
							type = 0;
						}
						else if (stype == "THAAD") {
							type = 1;
						}
					}
					else if (strcmp(var->Attribute("name"), "m_radius") == 0) {
						radius = atof(var->Attribute("value"));
					}
				}
				if (id != -1) {
					double lon = atof(inElem->Attribute("lon"));
					double lat = atof(inElem->Attribute("lat"));
					double height = atof(inElem->Attribute("height"));
					MisParameter mis;
					mis.id = id;
					mis.pos.x = lon;
					mis.pos.y = lat;
					mis.pos.z = height;
					mis.type = type;
					mis.radius = radius;
					m_misParameters.push_back(mis);
				}
			}
			else if (strcmp(inElem->Attribute("camp"), "blue") == 0) {
				int id = -1;
				double totalFlyTime, lnchTime, lon, lat, height;
				for (TiXmlElement *var = inElem->FirstChildElement(); var != NULL; var = var->NextSiblingElement()) {
					if (strcmp(var->Attribute("name"), "m_id") == 0) {
						id = atoi(var->Attribute("value"));
					}
					else if (strcmp(var->Attribute("name"), "m_totalFlyTime") == 0) {
						totalFlyTime = atof(var->Attribute("value"));
					}
					else if (strcmp(var->Attribute("name"), "m_lnchTime") == 0) {
						lnchTime = atof(var->Attribute("value"));
					}
					else if (strcmp(var->Attribute("name"), "m_fallSite") == 0) {
						string m_fallSite = var->Attribute("value");
						int pos = m_fallSite.find(",");
						lon = atof(m_fallSite.substr(0, pos).c_str());
						string b = m_fallSite.substr(pos + 1, m_fallSite.size());
						pos = b.find(",");
						lat = atof(b.substr(0, pos).c_str());
						height = atof(b.substr(pos + 1, b.size()).c_str());
					}
					
				}

				if (id != -1) {
					double lon1 = atof(inElem->Attribute("lon"));
					double lat1 = atof(inElem->Attribute("lat"));
					double height1 = atof(inElem->Attribute("height"));
					TarParameter tar;
					tar.id = id;
					tar.tarLnchPos.x = lon1;
					tar.tarLnchPos.y = lat1;
					tar.tarLnchPos.z = height1;
					tar.lnchTime = lnchTime;
					tar.totalFlyTime = totalFlyTime;
					tar.tarfallSite.x = lon;
					tar.tarfallSite.y = lat;
					tar.tarfallSite.z = height;					
					m_tarParameters.push_back(tar);
				}
			}
		}
	}
}

ReadInitFile::~ReadInitFile()
{
	delete m_doc;
}
