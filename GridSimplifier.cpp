
#include "stdafx.h"
#include "GridSimplifier.h"

GridSimplifier::GridSimplifier(char* lineFile, char* pointFile){
	map = readLines(lineFile);
	points = readPoints(pointFile);
	gridIndex = new GridTree(map, points);

	orig_size = gridIndex->size;
}

void GridSimplifier::wirteFile(string writeFile) {
	int length = map->lines.size() * 155 + gridIndex->size * 38;
	writeLines(map, writeFile, length);
}

bool GridSimplifier::removeS(Triangle &triangle){
	if (!gridIndex->hasPointInTri(&triangle)){
		triangle.p[1]->kept = false; //set point as removed
		triangle.p[2]->leftInd = triangle.p[0]->pointInd;
		triangle.p[0]->rightInd = triangle.p[2]->pointInd;
		--gridIndex->size; //remove point from index
		return true;
	}
	else
		return false;
}

void GridSimplifier::simplify(int limit){
	Triangle triangle;
	int last;
	int removed = 1, total_removed = 0;
	while (total_removed < limit && removed != 0){ //repeat the process if more points need to be removed
		for (int i = 0; i < map->lines.size(); ++i){ //for each line
			triangle.p[1] = map->lines[i]->points[0];
			last = map->lines[i]->points.size();
			if (map->lines[i]->points[last - 1]->leftInd == 0)
				continue; // it is already a segment
			triangle.p[2] = map->lines[i]->points[triangle.p[1]->rightInd];
			if (*(triangle.p[1]) == map->lines[i]->points[last - 1]){ //if the line is a cycle
				while (triangle.p[2]->rightInd < last){
					if (map->lines[i]->kept == 4) //leave at least 4 points
						break;
					triangle.p[1] = triangle.p[2];
					triangle.p[0] = map->lines[i]->points[triangle.p[1]->leftInd];
					triangle.p[2] = map->lines[i]->points[triangle.p[1]->rightInd];
					triangle.sort();
					removeS(triangle);
				}
			}
			else{
				while (triangle.p[2]->rightInd < last){
					triangle.p[1] = triangle.p[2];
					triangle.p[0] = map->lines[i]->points[triangle.p[1]->leftInd];
					triangle.p[2] = map->lines[i]->points[triangle.p[1]->rightInd];
					triangle.sort();
					removeS(triangle); //can I remove the current point?
				}
			}
		}
		removed = orig_size - gridIndex->size;
		total_removed += removed;
		orig_size = gridIndex->size;
	}
}

void GridSimplifier::simplifyT(int xl, int xr, int yl, int yr, const Rect& rect, Triangle& tri){
	for (int i = xl; i <= xr; ++i){
		for (int j = yl; j <= yr; ++j){
			for (int k = 0; k < gridIndex->grid[i][j].size(); ++k){
				if (gridIndex->grid[i][j][k]->lineInd != -1 && gridIndex->grid[i][j][k]->pointInd != 0 &&
					gridIndex->grid[i][j][k]->pointInd != map->lines[gridIndex->grid[i][j][k]->lineInd]->points.size() - 1){
					tri.p[1] = gridIndex->grid[i][j][k];
					tri.p[0] = map->lines[tri.p[1]->lineInd]->points[tri.p[1]->leftInd];
					tri.p[2] = map->lines[tri.p[1]->lineInd]->points[tri.p[1]->rightInd];
					tri.sort();
					if (tri.maxX <= rect.maxX && tri.maxY <= rect.maxY &&
						tri.minX >= rect.minX && tri.minY >= rect.minY) // skip those triangles acrossing boundary
						removeS(tri);
				}
			}
		}
	}
}

void GridSimplifier::simplifyTL(int index, Triangle& tri){
	for (int i = (index - 1) * map->lines.size() / 4; i < index * map->lines.size() / 4; ++i){
		for (int j = 1; j < map->lines[i]->points.size() - 1; ++j){
			tri.p[1] = map->lines[i]->points[j];
			tri.p[0] = map->lines[tri.p[1]->lineInd]->points[tri.p[1]->leftInd];
			tri.p[2] = map->lines[tri.p[1]->lineInd]->points[tri.p[1]->rightInd];
			tri.sort();
			removeS(tri);
		}
	}
}

void GridSimplifier::simplifyMT(int limit){
	Triangle tri1;
	Triangle tri2;
	Triangle tri3;
	Triangle tri4;
	
	Rect range = gridIndex->range;
	Rect rect1 = Rect(range.minX, (range.minX + range.maxX) / 2, range.minY, (range.minY + range.maxY) / 2);
	Rect rect2 = Rect(range.minX, (range.minX + range.maxX) / 2, (range.minY + range.maxY) / 2, range.maxY);
	Rect rect3 = Rect((range.minX + range.maxX) / 2, range.maxX, (range.minY + range.maxY) / 2, range.maxY);
	Rect rect4 = Rect((range.minX + range.maxX) / 2, range.maxX, range.minY, (range.minY + range.maxY) / 2);

	thread t0(&GridSimplifier::simplifyT, this, 0, (gridIndex->divideW - 1) / 2, 0, (gridIndex->divideH - 1) / 2, rect1, tri1);
	thread t1(&GridSimplifier::simplifyT, this, 0, (gridIndex->divideW - 1) / 2, (gridIndex->divideH - 1) / 2 + 1, gridIndex->divideH - 1, rect2, tri2);
	thread t2(&GridSimplifier::simplifyT, this, (gridIndex->divideW - 1) / 2 + 1, gridIndex->divideW - 1, (gridIndex->divideH - 1) / 2 + 1, gridIndex->divideH - 1, rect3, tri3);
	thread t3(&GridSimplifier::simplifyT, this, (gridIndex->divideW - 1) / 2 + 1, gridIndex->divideW - 1, 0, (gridIndex->divideH - 1) / 2, rect4, tri4);
	
/*
	thread t0(&GridSimplifier::simplifyTL, this, 1, tri1);
	thread t1(&GridSimplifier::simplifyTL, this, 2, tri2);
	thread t2(&GridSimplifier::simplifyTL, this, 3, tri3);
	thread t3(&GridSimplifier::simplifyTL, this, 4, tri4);
*/
	t0.join();
	t1.join();
	t2.join();
	t3.join();

}