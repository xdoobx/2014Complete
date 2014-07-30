#ifndef _GRIDTREEM_H_
#define _GRIDTREEM_H_

#include "GeoUtility.h"
#include <vector>

class GridTreeM{
private:
public:
	Rect range; //range of current branch node
	int num_point; // number of points under the node
	int divideW;
	double gridW;
	int divideH;
	double gridH;
	int threadN;// the number of useful threads. Here we set a fixed value. But later we should automatically set it based on the CPU.

	vector<Point*>*** gridM;

	//vector<Point*> gridM[100][100];

	int* sizes;

	//GridTreeM(LineSet* map, PointSet* points); //construct grid index with endpoints and constraint points

	GridTreeM(LineSetM* map, PointSetM* points); //construct grid index with endpoints and constraint points
	inline void insertLines(vector<Line*> lines, int threadId);
	inline const Point* insertM(Point* point, int tid);
	virtual inline bool hasPointInTri(const Triangle* triangle);
	virtual inline bool hasPointInPoly(const Polygon* polygon);
	virtual inline void PointsInPoly(const Polygon* polygon, vector<Point*>& p);

	int pointNumber();
};

#endif