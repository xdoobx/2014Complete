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

int GridTreeM::InferGridNumber(int lineN)
{
	int gridRow = 4 * lineN;
	return sqrt(gridRow);
}


GridTreeM::GridTreeM(LineSetM* map, PointSet* points){
	range = Rect(map->minx - 0.1, map->maxx + 0.1, map->miny - 0.1, map->maxy + 0.1);

	int num_point = 0;
	for (int i = 0; i < map->threadN; ++i){
		num_point += map->num_points[i];
	}

	divideH = divideW = sqrt(num_point-2*map->linesNumber())/2;

	gridW = (range.maxX - range.minX) / divideW;
	gridH = (range.maxY - range.minY) / divideH;

	for (int i = 0; i <= threadN; i++)
		sizes[i] = 0;

	gridM = new vector<Point*>**[divideH];
	for (int i = 0; i < divideH; ++i)
	{
		gridM[i] = new vector<Point*>*[divideW];

		for (int j = 0; j < divideW; j++)
			gridM[i][j] = new vector<Point*>[5];
	}

	//insert the points
	for (int i = 0; i < points->points.size(); ++i)
		insertM(points->points[i], 0);

	//insert the lines in parallel
	thread t0(&GridTreeM::insertLines, this, map->lines[0], 0);
	thread t1(&GridTreeM::insertLines, this, map->lines[1], 1);
	thread t2(&GridTreeM::insertLines, this, map->lines[2], 2);
	thread t3(&GridTreeM::insertLines, this, map->lines[3], 3);

	t0.join();
	t1.join();
	t2.join();
	t3.join();

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
	//gridM[gridX][gridY][tid].push_back(newPoint); // even though this point overlays a previous point
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

			//int vecN = threadN + 1;
			int vecN = threadN;
			for (int l = 0; l < vecN; l++)
			for (int k = 0; k < gridM[i][j][l].size(); ++k)
			{
				if (gridM[i][j][l][k]->kept && triangle->isInTri(gridM[i][j][l][k]->x, gridM[i][j][l][k]->y))
					return true;
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

			//int vecN = threadN + 1;
			int vecN = threadN;
			for (int l = 0; l < vecN; l++)
			for (int k = 0; k < gridM[i][j][l].size(); ++k)
			{
				if (gridM[i][j][l][k]->kept && polygon->isInPolygon(gridM[i][j][l][k]->x, gridM[i][j][l][k]->y))
					return true;
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

			//int vecN = threadN + 1;
			int vecN = threadN;
			for (int l = 0; l < vecN; l++)
			for (int k = 0; k < gridM[i][j][l].size(); ++k)
			{
				if (gridM[i][j][l][k]->kept && polygon->isInPolygon(gridM[i][j][l][k]->x, gridM[i][j][l][k]->y))
					p.push_back(gridM[i][j][l][k]);
			}
		}
	}
}

int GridTreeM::pointNumber()
{
	int sum = 0;
	for (int i = 0; i < threadN + 1; i++)
	{
		sum += sizes[i];
	}
	return sum;
}

