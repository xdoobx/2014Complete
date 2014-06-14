#ifndef _GRIDSIMPLIFIER_H_
#define _GRIDSIMPLIFIER_H_

#include "GridTree.h"

class GridSimplifier{
protected:
	GridTree* gridIndex; //index all points
	LineSet* map; //store lines
	PointSet* points; //store constraint points
	int orig_size; //number of unique points in line set
	inline bool removeS(Triangle &triangle); //simple version of remove point
public:
	GridSimplifier(char* lineFile, char* pointFile);
	//~Simplifier();
	void simplify(int limit); // use single processor
	void simplifyT(int xl, int xr, int yl, int yr, const Rect& rect, Triangle& tri); // in each thread
	void simplifyTL(int index, Triangle& tri);
	//void simplify2T(QuadTree* &root1, QuadTree* &root2, const Rect& rect, Triangle& tri);
	void simplifyMT(int limit); //main precess for multi-thread
	//void simplify5(int limit);
	void wirteFile(string writeFile);
};

#endif