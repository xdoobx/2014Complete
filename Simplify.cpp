// Simplify.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GridSimplifierM.h"
#include <iostream>
#include <time.h>
using namespace std;

int _tmain(int argc, char* argv[])
{
	clock_t total = clock(), begin = clock();
	clock_t end;

	//modify DEBUG->Properties->Configration Property->Character set, to "Use Multi-Byte Character Set"
	{
		GridSimplifierM simp((char*)argv[2], (char*)argv[3]);
		end = clock();
		cout << "construct: " << end - begin << endl;
		begin = end;
		simp.simplifyMTP(atoi(argv[1]));
		end = clock();
		cout << "Simplify: " << end - begin << endl;
		begin = end;
		simp.wirteFile((char*)argv[4]);
	}
	end = clock();
	cout << "write file: " << end - begin << endl;
	cout << "all\t" << end - total << endl;
	return 0;
}
