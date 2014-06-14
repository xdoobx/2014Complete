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
	//concurrency::concurrent_vector<Point*> points;
	vector<Point*> points;
	vector<int> shareEnd11;
	vector<int> shareEnd12;
	vector<int> shareEnd21;
	vector<int> shareEnd22;
	vector<int> shareEnd;

	/*~Line(){
		delete shareEnd11;
		delete shareEnd12;
		delete shareEnd21;
		delete shareEnd22;
		delete shareEnd;
		for (int i = 0; i < points.size(); ++i)
			delete points[i];
	}*/
};

struct LineSet
{
	const char* gmlLineString = ":<gml:LineString srsName=\"EPSG:54004\" xmlns:gml=\"http://www.opengis.net/gml\">";
	const char* gmlCoordinates = "<gml:coordinates decimal=\".\" cs=\",\" ts=\" \">";
	const char* endCoordinates = "</gml:coordinates>";
	const char* endLineString = "</gml:LineString>";
	vector<Line*> lines;
	double minX;
	double maxX;
	double minY;
	double maxY;

	void getShare(){ //scan sharing endpoint information, find 2-polyline-cycle
		for (int i = 0; i < lines.size(); ++i){
			for (int j = 0; j < lines[i]->shareEnd11.size(); ++j){
				for (int k = 0; k < lines[i]->shareEnd22.size(); ++k){
					if (lines[i]->shareEnd11[j] == lines[i]->shareEnd22[k])
						lines[i]->shareEnd.push_back(lines[i]->shareEnd11[j]);
				}
			}
			for (int j = 0; j < lines[i]->shareEnd12.size(); ++j){
				for (int k = 0; k < lines[i]->shareEnd21.size(); ++k){
					if (lines[i]->shareEnd12[j] == lines[i]->shareEnd21[k])
						lines[i]->shareEnd.push_back(lines[i]->shareEnd12[j]);
				}
			}
		}
	}
	/*~LineSet(){
		for (int i = 0; i < lines.size(); ++i)
			delete lines[i];
	}*/
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
		double prod1 = (x - p[1]->x)*(p[0]->y - y) - (x - p[0]->x)*(p[1]->y - y);
		double prod2 = (x - p[2]->x)*(p[0]->y - y) - (x - p[0]->x)*(p[2]->y - y);
		if (prod1 > 0 == prod2 > 0)
			if((prod1 != 0) == (prod2 != 0))
				return false;
			else{
				if ((x - p[2]->x)*(p[1]->y - y) == (x - p[1]->x)*(p[2]->y - y))
					return false;
				else
					return true;
			}
		else{
			double prod3 = (x - p[2]->x)*(p[1]->y - y) - (x - p[1]->x)*(p[2]->y - y);
			return prod2 > 0 != prod3 > 0 && (prod1 != 0 || prod3 != 0); // XOR used as !=
		}
		/*if (x <= minX || x >= maxX || y <= minY || y >= maxY){
			return false;
		}
		else
			return true;*/
	}
};

#endif