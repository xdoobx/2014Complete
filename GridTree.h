#ifndef _KDTREE_H_
#define _KDTREE_H_

#include "FileIO.h"
#include <thread>

class GridTree{
private:
public:
	Rect range; //range of current branch node
	int size; // number of points under the node
	int divideW;
	double gridW;
	int divideH;
	double gridH;

	vector<Point*>** grid;
	//concurrency::concurrent_vector<Point*>** grid; //store points in this grid
	//GridTree** subTree; //any index should guarantee they have 4 subtree under root node
	GridTree(LineSet* map, PointSet* points); //construct qtree with endpoints and constraint points
	//~QuadTree();

	inline const Point* insert(Point* point);
	virtual inline bool hasPointInTri(const Triangle* triangle);
};

#endif