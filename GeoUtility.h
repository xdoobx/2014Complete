#ifndef _GEOUTILITY_H_
#define _GEOUTILITY_H_

#include <vector>
#include <concurrent_vector.h>
using namespace std;

struct Point
{
	double x;
	double y;
	bool kept = true;
	int lineInd = -1;
	int pointInd = -1;
	int leftInd = -1;
	int rightInd = -1;
	Point(){}
	Point(double X, double Y) :x(X), y(Y){}
	inline bool operator==(const Point* point) const { return abs(x - point->x) < 0.001 && abs(y - point->y) < 0.001; }
	inline bool operator!=(const Point* point) const { return abs(x - point->x) >= 0.001 || abs(y - point->y) >= 0.001; }
	/*inline bool operator==(const Point* point) const { return x == point->x && y == point->y; }
	inline bool operator!=(const Point* point) const { return x != point->x || y != point->y; }*/
};

struct PointSet
{
	vector<Point*> points;
	double minX;
	double maxX;
	double minY;
	double maxY;
	/*~PointSet(){
		for (unsigned int i = 0; i < points.size(); ++i)
			delete points[i];
	}*/
};

struct Line
{
	long id;
	int kept = 0;
	bool cycle;
	bool share;
	//concurrency::concurrent_vector<Point*> points;
	vector<Point*> points;
};

struct LineSetM
{
	const static int threadN = 4;
	const char* gmlLineString = ":<gml:LineString srsName=\"EPSG:54004\" xmlns:gml=\"http://www.opengis.net/gml\">";
	const char* gmlCoordinates = "<gml:coordinates decimal=\".\" cs=\",\" ts=\" \">";
	const char* endCoordinates = "</gml:coordinates>";
	const char* endLineString = "</gml:LineString>";
	vector<Line*> lines[threadN];
	int num_points[threadN];
	double minXs[threadN];
	double maxXs[threadN];
	double minYs[threadN];
	double maxYs[threadN];

	double minx, maxx, miny, maxy;


	int linesNumber()
	{
		int sum = 0;
		for (int i = 0; i < threadN; i++){
			sum += lines[i].size();
		}
		return sum;
	}
};



struct Rect
{
	double maxX;
	double minX;
	double maxY;
	double minY;
	Rect(){}
	Rect(double x1, double x2, double y1, double y2) :
		minX(x1), maxX(x2), minY(y1), maxY(y2){}
	inline bool isInside(double x, double y){ 
		return x <= maxX && x > minX && y <= maxY && y > minY;
	}
	inline bool isInside(const Point* p){
		return p->x <= maxX && p->x > minX && p->y <= maxY && p->y > minY;
	}
};

struct Triangle
{
	Point* p[3];
	Triangle(){}
	Triangle(Point* P1, Point* P2, Point* P3){
		p[0] = P1;
		p[1] = P2;
		p[2] = P3;
		sort();
	}
	double maxX;
	double minX;
	double maxY;
	double minY;
	//only for VW
	double area;
	Triangle* pre;
	Triangle* next;
	int ind;
	//end only for vw

	//get x and y range of THIS triangle
	inline void sort(){
		if (p[0]->x < p[1]->x){
			if (p[0]->x < p[2]->x){
				minX = p[0]->x;
				if (p[1]->x > p[2]->x)
					maxX = p[1]->x;
				else
					maxX = p[2]->x;
			}
			else{
				minX = p[2]->x;
				maxX = p[1]->x;
			}
		}
		else{
			if (p[1]->x < p[2]->x){
				minX = p[1]->x;
				if (p[0]->x > p[2]->x)
					maxX = p[0]->x;
				else
					maxX = p[2]->x;
			}
			else{
				minX = p[2]->x;
				maxX = p[0]->x;
			}
		}
		if (p[0]->y < p[1]->y){
			if (p[0]->y < p[2]->y){
				minY = p[0]->y;
				if (p[1]->y > p[2]->y)
					maxY = p[1]->y;
				else
					maxY = p[2]->y;
			}
			else{
				minY = p[2]->y;
				maxY = p[1]->y;
			}
		}
		else{
			if (p[1]->y < p[2]->y){
				minY = p[1]->y;
				if (p[0]->y > p[2]->y)
					maxY = p[0]->y;
				else
					maxY = p[2]->y;
			}
			else{
				minY = p[2]->y;
				maxY = p[0]->y;
			}
		}
	}

	inline void calArea(){
		area = abs((p[2]->x - p[1]->x)*(p[0]->y - p[2]->y) - (p[2]->x - p[0]->x)*(p[1]->y - p[2]->y));
	}
	//whether a point is in this triangle
	inline bool isInTri(const double& x, const double& y) const {

		//if (x > maxX || x<minX || y>maxY || y<minY)
		//	return false;
		
		double prod1 = (x - p[1]->x)*(p[0]->y - y) - (x - p[0]->x)*(p[1]->y - y);
		double prod2 = (x - p[2]->x)*(p[0]->y - y) - (x - p[0]->x)*(p[2]->y - y);
		
		if (prod1 > 0 && prod2 > 0 || prod1 < 0 && prod2 < 0 || prod1 == 0 && prod2 == 0)
			return false;
		else{
			double prod3 = (x - p[2]->x)*(p[1]->y - y) - (x - p[1]->x)*(p[2]->y - y);
			if (prod3 == 0)
				return prod1 != 0 && prod2 != 0;
			if (prod2 > 0 && prod3 > 0 || prod2 < 0 && prod3 < 0)
				return false;
			if (prod2 != 0)
				return true;
			else
				return prod1 < 0 && prod3 < 0 || prod1 > 0 && prod3 > 0;
		}
		
	}
};

struct Polygon{
	int size;
	Point** p;
	Polygon(){}
	Polygon(int Size, Point** points){
		size = Size;
		p = points;
	}
	double maxX;
	double minX;
	double maxY;
	double minY;

	inline void getRange(){
		minX = maxX = p[0]->x;
		minY = maxY = p[0]->y;
		for (int i = 1; i < size; ++i){
			if (minX > p[i]->x)
				minX = p[i]->x;
			if (maxX < p[i]->x)
				maxX = p[i]->x;
			if (minY > p[i]->y)
				minY = p[i]->y;
			if (maxY < p[i]->y)
				maxY = p[i]->y;
		}
	}
	inline bool isInPolygon(const double& x, const double& y) const{
		if (x < minX || x > maxX || y < minY || y > maxY)
			return false;
		double count = 0;
		if (p[0]->x == x && p[0]->y == y)
			return false;
		for (int i = 0; i < size; ++i){
			int next = (i + 1) % size;
			if (p[next]->x == x && p[next]->y == y)
				return false;
			if (p[i]->y == p[next]->y){
				if (p[i]->y == y && p[i]->x < x != p[next]->x < x)
					return true;
				else
					continue;
			}
			else if (p[i]->y < p[next]->y){
				if (p[i]->y > y || p[next]->y < y)
					continue;
				else if (p[i]->y == y && p[i]->x > x || p[next]->y == y && p[next]->x > x){
					count += 0.5;
					continue;
				}
			}
			else{
				if (p[i]->y < y || p[next]->y > y)
					continue;
				else if (p[i]->y == y && p[i]->x > x || p[next]->y == y && p[next]->x > x){
					count -= 0.5;
					continue;
				}
			}
			if (p[i]->x < p[next]->x){
				if (p[next]->x < x)
					continue;
				else if (p[i]->x > x){
					++count;
					continue;
				}
			}
			else{
				if (p[i]->x < x)
					continue;
				else if (p[next]->x > x){
					++count;
					continue;
				}
			}
			double intersect = (p[next]->x*p[i]->y - p[next]->x*y - p[i]->x*p[next]->y + p[i]->x*y) / (p[i]->y - p[next]->y);
			if (intersect == x)
				return true;
			else if (intersect < x)
				continue;
			else
				++count;
		}
		if (int(count) % 2 == 0)
			return false;
		else
			return true;
	}
};

#endif