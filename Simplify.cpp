// Simplify.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GridSimplifierM.h"
#include <iostream>
#include <time.h>
using namespace std;

int _tmain(int argc, char* argv[])
{
	//modify DEBUG->Properties->Configration Property->Character set, to "Use Multi-Byte Character Set"
	{
		GridSimplifierM simp((char*)argv[2], (char*)argv[3]);
		simp.simplifyMTP(atoi(argv[1]));
		simp.wirteFile((char*)argv[4]);
	}
	return 0;
}
