#include "stdafx.h"
#include "GridTree.h"

GridTree::GridTree(LineSet* map, PointSet* points){
	range = Rect(map->minX - 0.1, map->maxX + 0.1, map->minY - 0.1, map->maxY + 0.1);
	divideW = 80;
	divideH = 80;
	gridW = (range.maxX - range.minX) / divideW;
	gridH = (range.maxY - range.minY) / divideH;
	//grid = new concurrency::concurrent_vector<Point*>*[divideW];
	grid = new vector<Point*>*[divideW];
	for (int i = 0; i < divideH; ++i)
		//grid[i] = new concurrency::concurrent_vector<Point*>[divideH];
		grid[i] = new vector<Point*>[divideH];
	size = 0;
	for (int i = 0; i < points->points.size(); ++i)
		insert(points->points[i]);



	for (int i = 0; i < map->lines.size(); ++i){
		for (int j = 0; j < map->lines[i]->points.size(); ++j){
			insert(map->lines[i]->points[j]);
		}
	}
}

const Point* GridTree::insert(Point* newPoint){
	int gridX = (newPoint->x - range.minX) / gridW;
	int gridY = (newPoint->y - range.minY) / gridH;
	grid[gridX][gridY].push_back(newPoint); // even though this point overlays a previous point
	++size;
	return newPoint;
}

bool GridTree::hasPointInTri(const Triangle* triangle){
	int gridMinX = (triangle->minX - range.minX) / gridW;
	int gridMinY = (triangle->minY - range.minY) / gridH;
	int gridMaxX = (triangle->maxX - range.minX) / gridW;
	int gridMaxY = (triangle->maxY - range.minY) / gridH;
	for (int i = gridMinX; i <= gridMaxX; ++i){
		for (int j = gridMinY; j <= gridMaxY; ++j){
			for (int k = 0; k < grid[i][j].size(); ++k){
				if (grid[i][j][k]->kept && triangle->isInTri(grid[i][j][k]->x, grid[i][j][k]->y))
					return true;
			}
		}
	}
	return false;
}

