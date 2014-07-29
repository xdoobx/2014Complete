#ifndef _GRIDSIMPLIFIERM_H_
#define _GRIDSIMPLIFIERM_H_

#include "GridTreeM.h"


class GridSimplifierM{
protected:
	GridTreeM* gridIndex; //index all points
	LineSetM* map; //store lines

	PointSetM* points; //store constraint points
	int orig_size; //number of unique points in line set
	inline bool removeS(Triangle &triangle, int threadId); //simple version of remove point
	inline bool removeS(Polygon &polygon, int threadId);
	inline void removeS(Polygon &polygon, vector<Point*>& p, int threadId);

	int threadN;//the maximal number of thread, should automatically set this number;
public:
	GridSimplifierM(char* lineFile, char* pointFile);
	//void simplify(int limit); // use single processor
	//void simplifyT(int xl, int xr, int yl, int yr, const Rect& rect, Triangle& tri); // in each thread

	void simplifyTP(vector<Line*> lines, Polygon& poly, int threadId);

	void simplifyMTP(int limit);
	void wirteFile(string writeFile);

};

#endif