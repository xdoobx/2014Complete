
#include "stdafx.h"
#include "GridSimplifierM.h"
#include "FileIO.h"

GridSimplifierM::GridSimplifierM(char* lineFile, char* pointFile){
	//clock_t begin, end;
	//begin = clock();
	map = readLinesM(lineFile);
	threadN = map->threadN;
	points = readPointsM(pointFile);
	//end = clock();
	//cout << "IO: " << end - begin << endl;

	//begin = clock();
	gridIndex = new GridTreeM(map, points);
	//end = clock();
	//cout << "index: " << end - begin << endl;
}

bool GridSimplifierM::removeS(Triangle &triangle, int threadId){
	if (!gridIndex->hasPointInTri(&triangle)){
		triangle.p[1]->kept = false; //set point as removed
		triangle.p[2]->leftInd = triangle.p[0]->pointInd;
		triangle.p[0]->rightInd = triangle.p[2]->pointInd;
		--gridIndex->sizes[threadId]; //remove point from index
		--map->lines[threadId][triangle.p[1]->lineInd]->kept;
		return true;
	}
	else
		return false;
}

bool GridSimplifierM::removeS(Polygon &poly, int threadId){
	vector<Point*> p;
	gridIndex->PointsInPoly(&poly, p);
	if (p.size() == 0){
		for (int i = 1; i < poly.size - 1; ++i)
			poly.p[i]->kept = false; //set point as removed
		poly.p[poly.size - 1]->leftInd = poly.p[0]->pointInd;
		poly.p[0]->rightInd = poly.p[poly.size - 1]->pointInd;
		gridIndex->sizes[threadId] -= poly.size - 2; //remove point from index
		map->lines[threadId][poly.p[0]->lineInd]->kept -= poly.size - 2;
		return true;
	}
	else{
		removeS(poly, p, threadId);
		return false;
	}
}

void GridSimplifierM::removeS(Polygon &poly, vector<Point*>& p, int threadId){
	int prev = 0, next = 2;
	for (int i = 1; i < poly.size - 1; ++i){
		bool can_delete = true;
		Triangle tri(poly.p[prev], poly.p[i],poly.p[next]);
		for (int j = 0; j < p.size(); ++j){
			if (tri.isInTri(p[j]->x, p[j]->y)){
				can_delete = false;
				break;
			}
		}
		if (can_delete){
			poly.p[prev]->rightInd = poly.p[i]->rightInd;
			poly.p[next]->leftInd = poly.p[i]->leftInd;
			--gridIndex->sizes[threadId];
			--map->lines[threadId][poly.p[0]->lineInd]->kept;
			poly.p[i]->kept = false;
		}
		else
			prev = i;
		++next;
	}
}

void GridSimplifierM::simplifyTP(vector<Line*> lines, Polygon& poly, int threadId){
	Triangle tri;
	for (int i = 0; i<lines.size(); ++i){
		
		if (lines[i]->cycle){
			for (int j = 1; j <= (int)(lines[i]->points.size() - 4) / 4; ++j){
				poly.p[1] = lines[i]->points[j * 4 - 3];
				poly.p[2] = lines[i]->points[j * 4 - 2];
				poly.p[3] = lines[i]->points[j * 4 - 1];
				poly.p[4] = lines[i]->points[j * 4];
				poly.p[0] = lines[i]->points[poly.p[1]->leftInd];
				poly.p[5] = lines[i]->points[poly.p[4]->rightInd];
				poly.getRange();

				removeS(poly, threadId);
			}
			int rest = lines[i]->kept;
			for (int j = 0; j < min(5, rest - 4); ++j){
				tri.p[1] = lines[i]->points[lines[i]->points.size() - j - 2];
				tri.p[0] = lines[i]->points[tri.p[1]->leftInd];
				tri.p[2] = lines[i]->points[tri.p[1]->rightInd];
				tri.sort();
				removeS(tri, threadId);
			}
		}
		else{
			for (int j = 1; j <= (int)(lines[i]->points.size() - 3) / 4; ++j){
				poly.p[1] = lines[i]->points[j * 4 - 3];
				poly.p[2] = lines[i]->points[j * 4 - 2];
				poly.p[3] = lines[i]->points[j * 4 - 1];
				poly.p[4] = lines[i]->points[j * 4];
				poly.p[0] = lines[i]->points[poly.p[1]->leftInd];
				poly.p[5] = lines[i]->points[poly.p[4]->rightInd];
				poly.getRange();

				removeS(poly, threadId);
			}
			for (int j = (lines[i]->points.size() - 2) % 4 == 0 && lines[i]->points.size() > 5 ? 5:(lines[i]->points.size() - 2) % 4 + 1;
				j > 2; --j){
				tri.p[1] = lines[i]->points[lines[i]->points.size() - j];
				tri.p[0] = lines[i]->points[tri.p[1]->leftInd];
				tri.p[2] = lines[i]->points[tri.p[1]->rightInd];
				tri.sort();
				removeS(tri, threadId);
			}
			if (lines[i]->kept > 3 || (lines[i]->share == false && lines[i]->kept > 2)){
				tri.p[1] = lines[i]->points[lines[i]->points.size() - 2];
				tri.p[0] = lines[i]->points[tri.p[1]->leftInd];
				tri.p[2] = lines[i]->points[tri.p[1]->rightInd];
				tri.sort();
				removeS(tri, threadId);
			}
		}
	}
}

void GridSimplifierM::simplifyMTP(int limit){
	Polygon* poly = new Polygon[threadN];
	for (int i = 0; i < threadN; ++i)
		poly[i] = Polygon(6, new Point*[6]);

	orig_size = gridIndex->pointNumber();

	thread *t = new thread[threadN];
	for (int i = 0; i < threadN; ++i)
		t[i] = thread(&GridSimplifierM::simplifyTP, this, map->lines[i], poly[i], i);
	for (int i = 0; i < threadN; ++i)
		t[i].join();

	int removed = orig_size - gridIndex->pointNumber();
}

void GridSimplifierM::wirteFile(string writeFile) {
	int length = map->linesNumber() * 155 + gridIndex->pointNumber() * 38;
	writeLinesM(map, writeFile, length);
}
