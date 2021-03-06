#include "DefendSystem.h"
#include "ReadInitFile.h"
#include "Vector.h"
#include "ExchangeData.h"
#include "Listener.h"
#include <iostream>
using namespace std;
int main(int argc, const char*argv[]){
	char fileName[] = "initFile.xml";
	ReadInitFile *read = new ReadInitFile(fileName);
	DefendSystem *system = new DefendSystem(read->m_misParameters,read->m_tarParameters);
	adevs::Simulator<IO_Type> sim(system);
	Listener* listener = new Listener();
	sim.addEventListener(listener);
	sim.execUntil(2000);
	return 0;
}
