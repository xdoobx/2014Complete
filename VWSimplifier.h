#ifndef _VWSIMPLIFIER_H_
#define _VWSIMPLIFIER_H_

#include "NaiveSimplifier.h"

class VWSimplifier : public NaiveSimplifier{
private:
	vector<Triangle*> tri_area[4];
	int heap_size[4];

	//update heap when a triangle is modified
	inline void heap_update(int heap, int index);

	//pop out the smallest triangle, also modify the indices after pop
	inline void heap_pop(int heap);
public:
	VWSimplifier(char* lineFile, char* pointFile);
	//void simplify(int limit); // use single processor
	void simplifyT(int heap, int limit); // in each thread
	void simplifyMT(int limit); //main precess for multi-thread
};

#endif