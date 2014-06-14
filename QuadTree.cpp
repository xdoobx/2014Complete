
#include "stdafx.h"
#include "QuadTree.h"

QuadTree::QuadTree(const double& minX, const double& maxX, const double& minY, const double& maxY)
{
	range = Rect(minX, maxX, minY, maxY);
	halfW = range.minX + (range.maxX - range.minX) / 2;
	halfH = range.minY + (range.maxY - range.minY) / 2;
	size = 0;
	point = NULL;
	subTree[0] = NULL;
	subTree[1] = NULL;
	subTree[2] = NULL;
	subTree[3] = NULL;
}

QuadTree::QuadTree(LineSet* map)
{
	range = Rect(map->minX - 0.1, map->maxX + 0.1, map->minY - 0.1, map->maxY + 0.1);
	halfW = (range.minX + range.maxX) / 2;
	halfH = (range.minY + range.maxY) / 2;
	size = 0;
	point = NULL;
	subTree[0] = NULL;
	subTree[1] = NULL;
	subTree[2] = NULL;
	subTree[3] = NULL;
	for (int i = 0; i < map->lines.size(); ++i){
		for (int j = 1; j < map->lines[i]->points.size() - 1; ++j){
			insert(map->lines[i]->points[j]);
		}
	}
	/*subTree[0] = new QuadTree(range.minX, halfW, range.minY, halfH);
	subTree[1] = new QuadTree(range.minX, halfW, halfH, range.maxY);
	subTree[2] = new QuadTree(halfW, range.maxX, halfH, range.maxY);
	subTree[3] = new QuadTree(halfW, range.maxX, range.minY, halfH);
	vector<Point*> branch[4];
	for (int i = 0; i < map->lines.size(); ++i){
		for (int j = 1; j < map->lines[i]->points.size() - 1; ++j){
			if (map->lines[i]->points[j]->x <= halfW){
				if (map->lines[i]->points[j]->y <= halfH)
					branch[0].push_back(map->lines[i]->points[j]);
				else
					branch[1].push_back(map->lines[i]->points[j]);
			}
			else{
				if (map->lines[i]->points[j]->y <= halfH)
					branch[3].push_back(map->lines[i]->points[j]);
				else
					branch[2].push_back(map->lines[i]->points[j]);
			}
		}
	}
	thread t0(&QuadTree::insertBranch, this, branch[0], subTree[0]);
	thread t1(&QuadTree::insertBranch, this, branch[1], subTree[1]);
	thread t2(&QuadTree::insertBranch, this, branch[2], subTree[2]);
	thread t3(&QuadTree::insertBranch, this, branch[3], subTree[3]);
	t0.join();
	t1.join();
	t2.join();
	t3.join();

	size = subTree[0]->size + subTree[1]->size +
		subTree[2]->size + subTree[3]->size;*/
}

QuadTree::QuadTree(LineSet* map, int mark)
{
	range = Rect(map->minX - 0.1, map->maxX + 0.1, map->minY - 0.1, map->maxY + 0.1);
	halfW = range.minX + (range.maxX - range.minX) / 2;
	halfH = range.minY + (range.maxY - range.minY) / 2;
	size = 0;
	point = NULL;
	subTree[0] = NULL;
	subTree[1] = NULL;
	subTree[2] = NULL;
	subTree[3] = NULL;
	for (int i = 0; i < map->lines.size(); ++i){
		insert(map->lines[i]->points[0]);
		insert(map->lines[i]->points[map->lines[i]->points.size() - 1]);
	}
}

QuadTree::QuadTree(LineSet* map, PointSet* points)
{
	range = Rect(map->minX - 0.1, map->maxX + 0.1, map->minY - 0.1, map->maxY + 0.1);
	halfW = range.minX + (range.maxX - range.minX) / 2;
	halfH = range.minY + (range.maxY - range.minY) / 2;
	size = 0;
	point = NULL;
	subTree[0] = NULL;
	subTree[1] = NULL;
	subTree[2] = NULL;
	subTree[3] = NULL;
	for (int i = 0; i < points->points.size(); ++i)
		insert(points->points[i]);
	const Point* temp;
	vector<int>* dup1, *dup2;
	for (int i = 0; i < map->lines.size(); ++i){ // find lines sharing endpoints
		temp = insert(map->lines[i]->points[0]); //try to insert head point
		if (temp != map->lines[i]->points[0] && temp->lineInd != i){ //a mate is already there
			if (temp->pointInd == 0){ //if it is a head point
				dup1 = &map->lines[temp->lineInd]->shareEnd11; //who's sharing endpoint with the head
				dup2 = &map->lines[temp->lineInd]->shareEnd12;
				for (int j = 0; j < dup1->size(); ++j){ //tell other head mates I'm here, also record them
					map->lines[dup1->at(j)]->shareEnd11.push_back(i);
					map->lines[i]->shareEnd11.push_back(dup1->at(j));
				}
				for (int j = 0; j < dup2->size(); ++j){ //do the same to the tail mates
					map->lines[dup2->at(j)]->shareEnd21.push_back(i);
					map->lines[i]->shareEnd12.push_back(dup2->at(j));
				}
				map->lines[temp->lineInd]->shareEnd11.push_back(i); //do it to the original mate
				map->lines[i]->shareEnd11.push_back(temp->lineInd);
			}
			else{ //or the mate is a tail
				dup1 = &map->lines[temp->lineInd]->shareEnd21; // who's sharing ep with the tail
				dup2 = &map->lines[temp->lineInd]->shareEnd22;
				for (int j = 0; j < dup1->size(); ++j){
					map->lines[dup1->at(j)]->shareEnd11.push_back(i);
					map->lines[i]->shareEnd11.push_back(dup1->at(j));
				}
				for (int j = 0; j < dup2->size(); ++j){
					map->lines[dup2->at(j)]->shareEnd21.push_back(i);
					map->lines[i]->shareEnd12.push_back(dup2->at(j));
				}
				map->lines[temp->lineInd]->shareEnd21.push_back(i);
				map->lines[i]->shareEnd12.push_back(temp->lineInd);
			}
		}
		temp = insert(map->lines[i]->points[map->lines[i]->points.size() - 1]); //try to insert tail
		if (temp != map->lines[i]->points[map->lines[i]->points.size() - 1]){
			if (temp->pointInd == 0){
				dup1 = &map->lines[temp->lineInd]->shareEnd11;
				dup2 = &map->lines[temp->lineInd]->shareEnd12;
				for (int j = 0; j < dup1->size(); ++j){
					map->lines[dup1->at(j)]->shareEnd12.push_back(i);
					map->lines[i]->shareEnd21.push_back(dup1->at(j));
				}
				for (int j = 0; j < dup2->size(); ++j){
					map->lines[dup2->at(j)]->shareEnd22.push_back(i);
					map->lines[i]->shareEnd22.push_back(dup2->at(j));
				}
				map->lines[temp->lineInd]->shareEnd12.push_back(i);
				map->lines[i]->shareEnd21.push_back(temp->lineInd);
			}
			else{
				dup1 = &map->lines[temp->lineInd]->shareEnd21;
				dup2 = &map->lines[temp->lineInd]->shareEnd22;
				for (int j = 0; j < dup1->size(); ++j){
					map->lines[dup1->at(j)]->shareEnd12.push_back(i);
					map->lines[i]->shareEnd21.push_back(dup1->at(j));
				}
				for (int j = 0; j < dup2->size(); ++j){
					map->lines[dup2->at(j)]->shareEnd22.push_back(i);
					map->lines[i]->shareEnd22.push_back(dup2->at(j));
				}
				map->lines[temp->lineInd]->shareEnd22.push_back(i);
				map->lines[i]->shareEnd22.push_back(temp->lineInd);
			}
		}
	}
	map->getShare(); // find the lines sharing both endpoints
}

QuadTree::QuadTree(PointSet* points)
{
	range = Rect(points->minX - 0.1, points->maxX + 0.1, points->minY - 0.1, points->maxY + 0.1);
	halfW = range.minX + (range.maxX - range.minX) / 2;
	halfH = range.minY + (range.maxY - range.minY) / 2;
	size = 0;
	point = NULL;
	subTree[0] = NULL;
	subTree[1] = NULL;
	subTree[2] = NULL;
	subTree[3] = NULL;
	for (int i = 0; i < points->points.size(); ++i)
		insert(points->points[i]);
}

//QuadTree::~QuadTree(){
//	if (subtree[0] != null){
//		delete subtree[0];
//		delete subtree[1];
//		delete subtree[2];
//		delete subtree[3];
//	}
//}

bool QuadTree::isCross(const Point* p1, const Point* p2,
	const double& p3x, const double& p3y, const double& p4x, const double& p4y){
	double prod1 = (p3x - p1->x)*(p2->y - p3y) - (p3x - p2->x)*(p1->y - p3y);
	double prod2 = (p4x - p1->x)*(p2->y - p4y) - (p4x - p2->x)*(p1->y - p4y);
	if ((prod1 > 0) == (prod2 > 0) || prod1 == 0 || prod2 == 0) // p1p2 divide p3p4
		return false;

	prod1 = (p1->x - p3x)*(p4y - p1->y) - (p1->x - p4x)*(p3y - p1->y);
	prod2 = (p2->x - p3x)*(p4y - p2->y) - (p2->x - p4x)*(p3y - p2->y);
	if ((prod1 > 0) == (prod2 > 0) || prod1 == 0 || prod2 == 0) //p3p4 divide p1p2
		return false;

	return true;
}

bool QuadTree::isIntersect(const Triangle* triangle){ //triangle * rectangle = 12
	return
		isCross(triangle->p[0], triangle->p[1], range.minX, range.minY, range.maxX, range.minY) ||
		isCross(triangle->p[0], triangle->p[2], range.minX, range.minY, range.maxX, range.minY) ||
		isCross(triangle->p[2], triangle->p[1], range.minX, range.minY, range.maxX, range.minY) ||
		isCross(triangle->p[0], triangle->p[1], range.maxX, range.minY, range.maxX, range.maxY) ||
		isCross(triangle->p[0], triangle->p[2], range.maxX, range.minY, range.maxX, range.maxY) ||
		isCross(triangle->p[2], triangle->p[1], range.maxX, range.minY, range.maxX, range.maxY) ||
		isCross(triangle->p[0], triangle->p[1], range.maxX, range.maxY, range.minX, range.maxY) ||
		isCross(triangle->p[0], triangle->p[2], range.maxX, range.maxY, range.minX, range.maxY) ||
		isCross(triangle->p[2], triangle->p[1], range.maxX, range.maxY, range.minX, range.maxY) ||
		isCross(triangle->p[0], triangle->p[1], range.minX, range.maxY, range.minX, range.minY) ||
		isCross(triangle->p[0], triangle->p[2], range.minX, range.maxY, range.minX, range.minY) ||
		isCross(triangle->p[2], triangle->p[1], range.minX, range.maxY, range.minX, range.minY);
}

void QuadTree::insertBranch(const vector<Point*>& points, QuadTree* branch){
	for (int i = 0; i < points.size(); ++i)
		branch->insert(points[i]);
}

const Point* QuadTree::insert (Point* newPoint){
	if (!range.isInside(newPoint))
		return NULL;
	if (size == 0){
		point = newPoint;
		size = 1;
		return newPoint;
	}
	if (point != NULL && *point == newPoint){ //return the existing point if conflicts
		return point;
	}
	if (subTree[0] == NULL)
		subDivid();

	const Point* p;
	if (newPoint->x <= halfW){
		if (newPoint->y <= halfH){
			if ((p = subTree[0]->insert(newPoint)) == newPoint){
				++size;
				return newPoint;
			}
			else if (p != NULL)
				return p;
		}
		else{
			if ((p = subTree[1]->insert(newPoint)) == newPoint){
				++size;
				return newPoint;
			}
			else if (p != NULL)
				return p;
		}
	}
	else{
		if (newPoint->y <= halfH){
			if ((p = subTree[3]->insert(newPoint)) == newPoint){
				++size;
				return newPoint;
			}
			else if (p != NULL)
				return p;
		}
		else{
			if ((p = subTree[2]->insert(newPoint)) == newPoint){
				++size;
				return newPoint;
			}
			else if (p != NULL)
				return p;
		}
	}
	return NULL;
}

/*iteration version insert is VERY SLOW*/
//inline const Point* QuadTree::insert(Point* newPoint){
//	stack<QuadTree*> stk;
//	QuadTree* cur = this;
//	stk.push(cur);
//	while (1){
//		while (cur->subTree[0] != NULL){
//			if (cur->subTree[0]->range.isInside(newPoint))
//				cur = cur->subTree[0];
//			else if (cur->subTree[1]->range.isInside(newPoint))
//				cur = cur->subTree[1];
//			else if (cur->subTree[2]->range.isInside(newPoint))
//				cur = cur->subTree[2];
//			else if (cur->subTree[3]->range.isInside(newPoint))
//				cur = cur->subTree[3];
//			else
//				return NULL;
//			stk.push(cur);
//		}
//		if (cur->point != NULL){
//			if (*cur->point == newPoint)
//				return cur->point;
//			else{
//				cur->subDivid();
//			}
//		}
//		else{
//			cur->point = newPoint;
//			while (!stk.empty()){
//				++stk.top()->size;
//				//cout << stk.size() << endl;
//				stk.pop();
//			}
//			return newPoint;
//		}
//	}
//}

bool QuadTree::subDivid(){
	subTree[0] = new QuadTree(range.minX, halfW, range.minY, halfH);
	subTree[1] = new QuadTree(range.minX, halfW, halfH, range.maxY);
	subTree[2] = new QuadTree(halfW, range.maxX, halfH, range.maxY);
	subTree[3] = new QuadTree(halfW, range.maxX, range.minY, halfH);
	//if (subTree[0]->insert(point) || subTree[1]->insert(point) ||
	//	subTree[2]->insert(point) || subTree[3]->insert(point)){
	//	point = NULL; //put point into lower layer, empty itself
	//	return true;
	//}
	if (point->x <= halfW){
		if (point->y <= halfH){
			subTree[0]->point = point;
			subTree[0]->size = 1;
			point = NULL;
			return true;
		}
		else{
			subTree[1]->point = point;
			subTree[1]->size = 1;
			point = NULL;
			return true;
		}
	}
	else{
		if (point->y <= halfH){
			subTree[3]->point = point;
			subTree[3]->size = 1;
			point = NULL;
			return true;
		}
		else{
			subTree[2]->point = point;
			subTree[2]->size = 1;
			point = NULL;
			return true;
		}
	}
	return false;
}

bool QuadTree::remove(const Point* newPoint){
	if (!range.isInside(newPoint) || size == 0) //out of range
		return false;

	if (subTree[0] != NULL){ //have children
		bool removed = false;
		if (newPoint->x <= halfW){
			if (newPoint->y <= halfH){
				if (subTree[0]->remove(newPoint)){
					--size;
					removed = true;
				}
			}
			else if (subTree[1]->remove(newPoint)){
				--size;
				removed = true;
			}
		}
		else{
			if (newPoint->y <= halfH){
				if (subTree[3]->remove(newPoint)){
					--size;
					removed = true;
				}
			}
			else if (subTree[2]->remove(newPoint)){
				--size;
				removed = true;
			}
		}
		if (removed){
			//remove successfully
			if (size == 1){ //need to recycle empty branches
				if (subTree[0]->point != NULL)
					point = subTree[0]->point;
				else if (subTree[1]->point != NULL)
					point = subTree[1]->point;
				else if (subTree[2]->point != NULL)
					point = subTree[2]->point;
				else if (subTree[3]->point != NULL)
					point = subTree[3]->point;
				//delete subTree[0];
				subTree[0] = NULL;
				//delete subTree[1];
				subTree[1] = NULL;
				//delete subTree[2];
				subTree[2] = NULL;
				//delete subTree[3];
				subTree[3] = NULL;
			}
			return true;
		}
	}
	else{
		point = NULL;
		size = 0;
		return true;
	}
	return false;
}

bool QuadTree::hasPointInTri(const Triangle* triangle){
	if (size == 0)
		return false;
	else if (size == 1)
		return triangle->isInTri(point->x, point->y);
	else if (triangle->maxX < range.minX || triangle->minX > range.maxX ||
		triangle->maxY < range.minY || triangle->minY > range.maxY) // out of range
		return false;
	else if(!range.isInside(triangle->p[0]) && !range.isInside(triangle->p[1]) && !range.isInside(triangle->p[2]) && 
		!triangle->isInTri(range.maxX, range.maxY) && !triangle->isInTri(range.minX, range.minY) && // triangle point in rec
		!triangle->isInTri(range.maxX, range.minY) && !triangle->isInTri(range.minX, range.maxY) && // rec point in triangle
		!isIntersect(triangle)) //outside each other
		return false;
	else{ //intersect and have more than 1 points
		return subTree[0]->hasPointInTri(triangle) || subTree[1]->hasPointInTri(triangle) ||
			subTree[2]->hasPointInTri(triangle) || subTree[3]->hasPointInTri(triangle);
	}
}

void QuadTree::pointInTri(const Triangle* triangle, vector<Point*>* v){
	if (size == 0)
		return;
	else if (size == 1){
		if (point->kept && triangle->isInTri(point->x, point->y))
			v->push_back(point);
		return;
	}
	else if (triangle->maxX < range.minX || triangle->minX > range.maxX ||
		triangle->maxY < range.minY || triangle->minY > range.maxY) // out of range
		return;
	else if (!range.isInside(triangle->p[0]) && !range.isInside(triangle->p[1]) && !range.isInside(triangle->p[2]) &&
		!triangle->isInTri(range.maxX, range.maxY) && !triangle->isInTri(range.minX, range.minY) && // triangle point in rec
		!triangle->isInTri(range.maxX, range.minY) && !triangle->isInTri(range.minX, range.maxY) && // rec point in triangle
		!isIntersect(triangle)){ //their segments do not intersect
		return;
	}
	else{ //intersect and have more than 1 points
		subTree[0]->pointInTri(triangle, v);
		subTree[1]->pointInTri(triangle, v);
		subTree[2]->pointInTri(triangle, v);
		subTree[3]->pointInTri(triangle, v);
	}
}