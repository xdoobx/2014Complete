
#include "stdafx.h"
#include "GridSimplifierM.h"
#include "FileIO.h"

GridSimplifierM::GridSimplifierM(char* lineFile, char* pointFile){
	map = readLinesM(lineFile);
	threadN = map->threadN;
	points = readPointsM(pointFile);
	gridIndex = new GridTreeM(map, points);
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
	if (!gridIndex->hasPointInPoly(&poly)){
		for (int i = 1; i < poly.size - 1; ++i)
			poly.p[i]->kept = false; //set point as removed
		poly.p[poly.size - 1]->leftInd = poly.p[0]->pointInd;
		poly.p[0]->rightInd = poly.p[poly.size - 1]->pointInd;
		gridIndex->sizes[threadId] -= poly.size - 2; //remove point from index
		map->lines[threadId][poly.p[0]->lineInd]->kept -= poly.size - 2;
		return true;
	}
	else{
		int prev = 0, next = 2;
		for (int i = 1; i < poly.size - 1; ++i){
			Triangle tri(poly.p[prev], poly.p[i], poly.p[next]);
			if (!removeS(tri, threadId))
				prev = i;
			++next;
		}
		return false;
	}
}

void GridSimplifierM::simplifyTP(vector<Line*> &lines, Polygon& poly, int threadId){
	Triangle tri;
	int remove_size = poly.size - 2;
	for (int i = 0; i<lines.size(); ++i){
		if (lines[i]->cycle){
			for (int j = 1; j <= (int)(lines[i]->points.size() - 4) / remove_size; ++j){
				for (int k = 1; k <= remove_size; ++k)
					poly.p[k] = lines[i]->points[j*remove_size - 3 + k];
				poly.p[0] = lines[i]->points[poly.p[1]->leftInd];
				poly.p[remove_size + 1] = lines[i]->points[poly.p[remove_size]->rightInd];
				poly.getRange();
				removeS(poly, threadId);
			}
			int rest = lines[i]->kept;
			int togo = lines[i]->points.size() % remove_size + remove_size;
			int left_count = togo < rest - 4 ? togo : rest - 4;
			for (int j = 0; j < left_count; ++j){
				tri.p[1] = lines[i]->points[lines[i]->points.size() - j - 2];
				tri.p[0] = lines[i]->points[tri.p[1]->leftInd];
				tri.p[2] = lines[i]->points[tri.p[1]->rightInd];
				tri.sort();
				removeS(tri, threadId);
			}
		}
		else{
			for (int j = 1; j <= (int)(lines[i]->points.size() - 3) / remove_size; ++j){
				for (int k = 1; k <= remove_size; ++k)
					poly.p[k] = lines[i]->points[j*remove_size - remove_size + k];
				poly.p[0] = lines[i]->points[poly.p[1]->leftInd];
				poly.p[remove_size + 1] = lines[i]->points[poly.p[remove_size]->rightInd];
				poly.getRange();
				removeS(poly, threadId);
			}
			for (int j = (lines[i]->points.size() - 2) % remove_size == 0 && lines[i]->points.size() > remove_size + 1 ?
				remove_size + 1 : (lines[i]->points.size() - 2) % remove_size + 1;
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
	int poly_size = sqrt(gridIndex->num_point / map->linesNumber()) + 2;

	for (int i = 0; i < threadN; ++i)
		poly[i] = Polygon(poly_size, new Point*[poly_size]);

	orig_size = gridIndex->pointNumber();

	thread *t = new thread[threadN];
	for (int i = 0; i < threadN; ++i)
		t[i] = thread(&GridSimplifierM::simplifyTP, this, map->lines[i], poly[i], i);
	for (int i = 0; i < threadN; ++i)
		t[i].join();

	int removed = orig_size - gridIndex->pointNumber();
	if (removed < limit){
		Triangle* tri = new Triangle[threadN];
		while (removed < limit && removed != 0){
			orig_size = gridIndex->pointNumber();
			for (int i = 0; i < threadN; ++i)
				t[i] = thread(&GridSimplifierM::simplifyT, this, map->lines[i], tri[i], i);
			for (int i = 0; i < threadN; ++i)
				t[i].join();
			removed = orig_size - gridIndex->pointNumber();
		}
	}
	/*Triangle* tri = new Triangle[threadN];
	thread *t = new thread[threadN];
	orig_size = gridIndex->pointNumber();
	for (int i = 0; i < threadN; ++i)
		t[i] = thread(&GridSimplifierM::simplifyT, this, map->lines[i], tri[i], i);
	for (int i = 0; i < threadN; ++i)
		t[i].join();
	int removed = orig_size - gridIndex->pointNumber();
	while (removed < limit && removed != 0){
		orig_size = gridIndex->pointNumber();
		for (int i = 0; i < threadN; ++i)
			t[i] = thread(&GridSimplifierM::simplifyT, this, map->lines[i], tri[i], i);
		for (int i = 0; i < threadN; ++i)
			t[i].join();
		removed = orig_size - gridIndex->pointNumber();
	}*/
}
void GridSimplifierM::simplifyT(vector<Line*> &lines, Triangle& tri, int threadId){
	for (int i = 0; i<lines.size(); ++i){
		if (lines[i]->cycle){
			tri.p[0] = lines[i]->points[0];
			while (lines[i]->kept > 4 && tri.p[0]->rightInd != lines[i]->points.size() - 1){
				tri.p[1] = lines[i]->points[tri.p[0]->rightInd];
				tri.p[2] = lines[i]->points[tri.p[1]->rightInd];
				tri.sort();
				if(!removeS(tri, threadId))
					tri.p[0] = lines[i]->points[tri.p[0]->rightInd];
			}
			int rest = lines[i]->kept;
			int left_count = 2 < rest - 4 ? 2 : rest - 4;
			tri.p[2] = lines[i]->points[lines[i]->points.size() - 1];
			for (int j = 0; j < left_count; ++j){
				tri.p[1] = lines[i]->points[tri.p[2]->leftInd];
				tri.p[0] = lines[i]->points[tri.p[1]->leftInd];
				tri.sort();
				if(!removeS(tri, threadId))
					tri.p[2] = lines[i]->points[tri.p[2]->leftInd];
			}
		}
		else{
			tri.p[0] = lines[i]->points[0];
			while (lines[i]->kept > 3 && tri.p[0]->rightInd != lines[i]->points.size() - 1){
				tri.p[1] = lines[i]->points[tri.p[0]->rightInd];
				tri.p[2] = lines[i]->points[tri.p[1]->rightInd];
				tri.sort();
				if(!removeS(tri, threadId))
					tri.p[0] = lines[i]->points[tri.p[0]->rightInd];
			}
			if (lines[i]->kept > 3 || (lines[i]->share == false && lines[i]->kept > 2)){
				tri.p[2] = lines[i]->points[lines[i]->points.size() - 1];
				tri.p[1] = lines[i]->points[tri.p[2]->leftInd];
				tri.p[0] = lines[i]->points[tri.p[1]->leftInd];
				tri.sort();
				removeS(tri, threadId);
			}
		}
	}
}

void GridSimplifierM::wirteFile(string writeFile) {
	int length = map->linesNumber() * 161 + gridIndex->pointNumber() * 28;
	writeLinesM(map, writeFile, length);
}
