#pragma once
#include "Vector.h"
#include "ExchangeData.h"
#include "tinystr.h"
#include "tinyxml.h"
#include<list>
#include <iostream>
using namespace std;

class ReadInitFile
{
public:
	ReadInitFile(char fileName[]);

	~ReadInitFile();

public:
	list<TarParameter> m_tarParameters;
	list<MisParameter> m_misParameters;
	TiXmlDocument *m_doc;
private:
	void readFile();

};

