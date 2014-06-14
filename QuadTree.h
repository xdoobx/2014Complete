#ifndef _QUADTREE_H_
#define _QUADTREE_H_

#include "FileIO.h"
#include <stack>
#include <thread>

class QuadTree{

private:
	inline bool subDivid();
	inline bool isCross(const Point* p1, const Point* p2,
		const double& p3x, const double& p3y, const double& p4x, const double& p4y);
	inline bool isIntersect(const Triangle* triangle);
public:
	double halfW;
	double halfH;
	Rect range; //range of current branch node
	int size; // number of points under the node

	Point* point; //if it is a leaf node, it should record a point, otherwise null
	QuadTree* subTree[4]; //any index should guarantee they have 4 subtree under root node

	QuadTree(const double& minX, const double& maxX, const double& minY, const double& maxY);//construct empty qtree
	QuadTree(LineSet* map); //construct qtree with inner points in a map
	QuadTree(LineSet* map, int mark); //construct qtree with endpoints in a map
	QuadTree(LineSet* map, PointSet* points); //construct qtree with endpoints and constraint points
	QuadTree(PointSet* points); //construct qtree with constraint points
	//~QuadTree();
	void insertBranch(const vector<Point*>& points, QuadTree* branch);
	inline const Point* insert(Point* point);
	virtual inline bool hasPointInTri(const Triangle* triangle);
	virtual inline void pointInTri(const Triangle* triangle, vector<Point*>* v);
	virtual inline bool remove(const Point* point);
};

#endif