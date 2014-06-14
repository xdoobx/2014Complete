#ifndef _NAIVESIMPLIFIER_H_
#define _NAIVESIMPLIFIER_H_

#include "QuadTree.h"

class NaiveSimplifier{
protected:
	QuadTree* qTreeLine; //index removable points
	QuadTree* qTreePoint; //index static points (constraint points + line endpoints)
	LineSet* map; //store lines
	PointSet* points; //store constraint points
	int orig_size; //number of unique points in line set
	inline bool removeP(Triangle &triangle); //remove a point in the given triangle
	inline bool removeS(Triangle &triangle); //simple version of remove point
public:
	NaiveSimplifier(char* lineFile, char* pointFile);
	//~Simplifier();
	void simplify(int limit); // use single processor
	void simplifyT(QuadTree* &root, const Rect& rect, Triangle& tri); // in each thread
	void simplify2T(QuadTree* &root1, QuadTree* &root2, const Rect& rect, Triangle& tri);
	void simplifyMT(int limit); //main precess for multi-thread
	//void simplify5(int limit);
	void wirteFile(string writeFile);
};

#endif