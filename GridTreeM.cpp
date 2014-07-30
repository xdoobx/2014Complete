#include "stdafx.h"
#include "GridTreeM.h"
#include <thread>



void GridTreeM::insertLines(vector<Line*> lines, int threadId){
	sizes[threadId] = 0;
	for (int i = 0; i < lines.size(); ++i){
		for (int j = 0; j < lines[i]->points.size(); ++j){
			insertM(lines[i]->points[j], threadId);
		}
	}
}

GridTreeM::GridTreeM(LineSetM* map, PointSetM* points){
	range = Rect(min(map->minx, points->minX) - 0.1, max(map->maxx, points->maxX) + 0.1,
		min(map->miny, points->minY) - 0.1, max(map->maxy, points->maxY) + 0.1);
	threadN = map->threadN;
	sizes = new int[threadN];
	num_point = 0;
	for (int i = 0; i < map->threadN; ++i){
		num_point += map->num_points[i];
		num_point += points->point[i].size();
	}

	divideH = divideW = sqrt(num_point)/3;

	gridW = (range.maxX - range.minX) / divideW;
	gridH = (range.maxY - range.minY) / divideH;

	for (int i = 0; i < threadN; i++)
		sizes[i] = 0;

	gridM = new vector<Point*>**[divideH];
	for (int i = 0; i < divideH; ++i)
	{
		gridM[i] = new vector<Point*>*[divideW];

		for (int j = 0; j < divideW; j++)
			gridM[i][j] = new vector<Point*>[threadN];
	}

	//insert the points
	for (int i = 0; i < points->threadN; ++i){
		for (int j = 0; j < points->point[i].size(); ++j)
			insertM(points->point[i][j], i);
	}

	//insert the lines in parallel
	thread* t = new thread[threadN];
	for (int i = 0; i < threadN; ++i)
		t[i] = thread(&GridTreeM::insertLines, this, map->lines[i], i);
	for (int i = 0; i < threadN; ++i)
		t[i].join();

	for (int i = 0; i < threadN; i++){
		for (int j = 0; j < map->lines[i].size(); ++j){
			if (map->lines[i][j]->share == false){
				Point* end1 = map->lines[i][j]->points[0];
				int gridx = (end1->x - range.minX) / gridW;
				int gridy = (end1->y - range.minY) / gridH;
				for (int k = 0; k < threadN; ++k){
					for (int l = 0; l < gridM[gridx][gridy][k].size(); ++l){
						if (*gridM[gridx][gridy][k][l] == end1){
							int line_size = map->lines[k][gridM[gridx][gridy][k][l]->lineInd]->points.size();
							if ((k != i || j != gridM[gridx][gridy][k][l]->lineInd) &&
								*map->lines[k][gridM[gridx][gridy][k][l]->lineInd]->
								points[line_size - 1 - gridM[gridx][gridy][k][l]->pointInd] ==
								map->lines[i][j]->points[map->lines[i][j]->points.size() - 1]){
								map->lines[i][j]->share = true;
								map->lines[k][gridM[gridx][gridy][k][l]->lineInd]->share = true;
							}
						}
					}
				}
			}
		}
	}
}

const Point* GridTreeM::insertM(Point* newPoint,int tid){
	int gridX = (newPoint->x - range.minX) / gridW;
	int gridY = (newPoint->y - range.minY) / gridH;
	gridM[gridX][gridY][tid].push_back(newPoint); // even though this point overlays a previous point
	++sizes[tid];
	return newPoint;
}

bool GridTreeM::hasPointInTri(const Triangle* triangle){
	int gridMinX = (triangle->minX - range.minX) / gridW;
	int gridMinY = (triangle->minY - range.minY) / gridH;
	int gridMaxX = (triangle->maxX - range.minX) / gridW;
	int gridMaxY = (triangle->maxY - range.minY) / gridH;
	for (int i = gridMinX; i <= gridMaxX; ++i){
		for (int j = gridMinY; j <= gridMaxY; ++j){
			for (int l = 0; l < threadN; l++){
				for (int k = 0; k < gridM[i][j][l].size(); ++k)
				{
					if (gridM[i][j][l][k]->kept && triangle->isInTri(gridM[i][j][l][k]->x, gridM[i][j][l][k]->y))
						return true;
				}
			}
		}
	}
	return false;
}

bool GridTreeM::hasPointInPoly(const Polygon* polygon){
	int gridMinX = (polygon->minX - range.minX) / gridW;
	int gridMinY = (polygon->minY - range.minY) / gridH;
	int gridMaxX = (polygon->maxX - range.minX) / gridW;
	int gridMaxY = (polygon->maxY - range.minY) / gridH;
	for (int i = gridMinX; i <= gridMaxX; ++i){
		for (int j = gridMinY; j <= gridMaxY; ++j){
			for (int l = 0; l < threadN; l++){
				for (int k = 0; k < gridM[i][j][l].size(); ++k)
				{
					if (gridM[i][j][l][k]->kept && polygon->isInPolygon(gridM[i][j][l][k]->x, gridM[i][j][l][k]->y))
						return true;
				}
			}
		}
	}
	return false;
}

void GridTreeM::PointsInPoly(const Polygon* polygon, vector<Point*>& p){
	int gridMinX = (polygon->minX - range.minX) / gridW;
	int gridMinY = (polygon->minY - range.minY) / gridH;
	int gridMaxX = (polygon->maxX - range.minX) / gridW;
	int gridMaxY = (polygon->maxY - range.minY) / gridH;
	for (int i = gridMinX; i <= gridMaxX; ++i){
		for (int j = gridMinY; j <= gridMaxY; ++j){
			for (int l = 0; l < threadN; l++){
				for (int k = 0; k < gridM[i][j][l].size(); ++k)
				{
					if (gridM[i][j][l][k]->kept && polygon->isInPolygon(gridM[i][j][l][k]->x, gridM[i][j][l][k]->y))
						p.push_back(gridM[i][j][l][k]);
				}
			}
		}
	}
}

int GridTreeM::pointNumber()
{
	int sum = 0;
	for (int i = 0; i < threadN; i++)
	{
		sum += sizes[i];
	}
	return sum;
}

