
#include "stdafx.h"
#include "NaiveSimplifier.h"

NaiveSimplifier::NaiveSimplifier(char* lineFile, char* pointFile){
	LineSet* Map = readLines(lineFile);
	PointSet* Points = readPoints(pointFile);
	qTreeLine = new QuadTree(Map);
	qTreePoint = new QuadTree(Map, Points);

	map = Map;
	points = Points;
	orig_size = qTreeLine->size;
}

void NaiveSimplifier::wirteFile(string writeFile) {
	int length = map->lines.size() * 155 + (qTreeLine->size + 2 * map->lines.size()) * 38;
	writeLines(map, writeFile, length);
}

bool NaiveSimplifier::removeP(Triangle &triangle){
	if (!qTreePoint->hasPointInTri(&triangle)){ //is there any city in this triangle?
		if (!qTreeLine->hasPointInTri(&triangle)){ //is there any line-points in this triangle?
			if (map->lines[triangle.p[1]->lineInd]->kept == 3){ //is it the last point on the line?
				for (int i = 0; i < map->lines[triangle.p[1]->lineInd]->shareEnd.size(); ++i){
					if (map->lines[map->lines[triangle.p[1]->lineInd]->shareEnd.at(i)]->kept == 2)
						return false; //Oops, sharing both endpoints with a segment
				}
			}
			--map->lines[triangle.p[1]->lineInd]->kept; //line point number decrease
			triangle.p[1]->kept = false; //set point as removed
			triangle.p[2]->leftInd = triangle.p[0]->pointInd;
			triangle.p[0]->rightInd = triangle.p[2]->pointInd;
			qTreeLine->remove(triangle.p[1]); //remove point from index
			return true;
		}
		else{ //try to remove restricting line points first
			vector<Point*>* constraint = new vector<Point*>;
			qTreeLine->pointInTri(&triangle, constraint);
			Triangle tri;
			Point* toRm;
			bool success = true;
			for (int i = 0; i < constraint->size(); ++i){ //remove each point
				toRm = constraint->at(i);
				tri.p[0] = map->lines[toRm->lineInd]->points[toRm->leftInd];
				tri.p[1] = toRm;
				tri.p[2] = map->lines[toRm->lineInd]->points[toRm->rightInd];
				tri.sort();
				if (!removeP(tri)){
					success = false;
					break;
				}
			}
			//after delete constraint
			if (success){
				if (map->lines[triangle.p[1]->lineInd]->kept == 3){
					for (int i = 0; i < map->lines[triangle.p[1]->lineInd]->shareEnd.size(); ++i){
						if (map->lines[map->lines[triangle.p[1]->lineInd]->shareEnd[i]]->kept == 2)
							return false;
					}
				}
				--map->lines[triangle.p[1]->lineInd]->kept;
				triangle.p[1]->kept = false;
				triangle.p[2]->leftInd = triangle.p[0]->pointInd;
				triangle.p[0]->rightInd = triangle.p[2]->pointInd;
				qTreeLine->remove(triangle.p[1]);
				return true;
			}
		}
	}
	return false;
}

bool NaiveSimplifier::removeS(Triangle& triangle){
	if (!qTreePoint->hasPointInTri(&triangle)){ //is there any city in this triangle?
		if (!qTreeLine->hasPointInTri(&triangle)){ //is there any points on lines in this triangle?
			if (map->lines[triangle.p[1]->lineInd]->kept < 7){ //is it the last point on the line?
				for (int i = 0; i < map->lines[triangle.p[1]->lineInd]->shareEnd.size(); ++i){
					if (map->lines[map->lines[triangle.p[1]->lineInd]->shareEnd[i]]->kept < 6)
						return false; //Oops, sharing both endpoints with a segment
				}
			}
			--map->lines[triangle.p[1]->lineInd]->kept; //line point number decrease
			triangle.p[1]->kept = false; //set point as removed
			triangle.p[2]->leftInd = triangle.p[0]->pointInd;
			triangle.p[0]->rightInd = triangle.p[2]->pointInd;
			qTreeLine->remove(triangle.p[1]); //remove point from index
			return true;
		}
		else
			return false;
	}
	return false;
}

void NaiveSimplifier::simplify(int limit){
	Triangle triangle;
	int last;
	int removed = 1, total_removed = 0;
	while(total_removed < limit && removed != 0){ //repeat the process if more points need to be removed
		for (int i = 0; i < map->lines.size(); ++i){ //for each line
			triangle.p[1] = map->lines[i]->points[0];
			last = map->lines[i]->points.size();
			if (map->lines[i]->points[last - 1]->leftInd == 0)
				continue; // it is already a segment
			triangle.p[2] = map->lines[i]->points[triangle.p[1]->rightInd];
			if (*(triangle.p[1]) == map->lines[i]->points[last-1]){ //if the line is a cycle
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
		removed = orig_size - qTreeLine->size;
		total_removed += removed;
		orig_size = qTreeLine->size;
	}
}

/* Recursively remove points from Quadtree */
void NaiveSimplifier::simplifyT(QuadTree* &root, const Rect& rect, Triangle& tri){
	if (root->point == NULL){
		if (root->subTree[0] != NULL)
			simplifyT(root->subTree[0], rect, tri);
		if (root != NULL){
			if (root->subTree[1] != NULL)
				simplifyT(root->subTree[1], rect, tri);
			if (root != NULL){
				if (root->subTree[2] != NULL)
					simplifyT(root->subTree[2], rect, tri);
				if (root != NULL){
					if (root->subTree[3] != NULL)
						simplifyT(root->subTree[3], rect, tri);
				}
			}
		}
	}
	if (root != NULL && root->point != NULL) {
		if (map->lines[root->point->lineInd]->cycle && map->lines[root->point->lineInd]->kept == 4)
			return; //if it is a cycle, leave 4 at least
		tri.p[0] = map->lines[root->point->lineInd]->points[root->point->leftInd];
		tri.p[1] = root->point;
		tri.p[2] = map->lines[root->point->lineInd]->points[root->point->rightInd];
		tri.sort();
		if (tri.maxX <= rect.maxX && tri.maxY <= rect.maxY &&
			tri.minX >= rect.minX && tri.minY >= rect.minY) // skip those triangles acrossing boundary
			removeS(tri);
	}
}

void NaiveSimplifier::simplify2T(QuadTree* &root1, QuadTree* &root2, const Rect& rect, Triangle& tri){
	simplifyT(root1, rect, tri);
	simplifyT(root2, rect, tri);
}

/*Divide main process into four threads. Triangle crossing quadtree boundary is processed at last*/
void NaiveSimplifier::simplifyMT(int limit){
	if (qTreeLine->subTree[0] != NULL){
		/*Triangle tri1;
		Triangle tri2;
		thread t0(&NaiveSimplifier::simplify2T, this, qTreeLine->subTree[0], qTreeLine->subTree[1],
			Rect(qTreeLine->range.minX, qTreeLine->subTree[0]->range.maxX,
			qTreeLine->range.minY, qTreeLine->range.maxY), tri1);
		thread t1(&NaiveSimplifier::simplify2T, this, qTreeLine->subTree[2], qTreeLine->subTree[3],
			Rect(qTreeLine->subTree[0]->range.maxX, qTreeLine->range.maxX,
			qTreeLine->range.minY, qTreeLine->range.maxY), tri2);

		t0.join();
		t1.join();
		qTreeLine->size = qTreeLine->subTree[0]->size + qTreeLine->subTree[1]->size +
			qTreeLine->subTree[2]->size + qTreeLine->subTree[3]->size;
		if (orig_size - qTreeLine->size >= limit)
			return;*/
		
		Triangle tri1;
		Triangle tri2;
		Triangle tri3;
		Triangle tri4;
		thread t0(&NaiveSimplifier::simplifyT, this, qTreeLine->subTree[0], qTreeLine->subTree[0]->range, tri1);
		thread t1(&NaiveSimplifier::simplifyT, this, qTreeLine->subTree[1], qTreeLine->subTree[1]->range, tri2);
		thread t2(&NaiveSimplifier::simplifyT, this, qTreeLine->subTree[2], qTreeLine->subTree[2]->range, tri3);
		thread t3(&NaiveSimplifier::simplifyT, this, qTreeLine->subTree[3], qTreeLine->subTree[3]->range, tri4);
		
		t0.join();
		t1.join();
		t2.join();
		t3.join();
		// the root size was messed up by multi-thread
		qTreeLine->size = qTreeLine->subTree[0]->size + qTreeLine->subTree[1]->size +
			qTreeLine->subTree[2]->size + qTreeLine->subTree[3]->size;
		if (orig_size - qTreeLine->size >= limit)
			return;

		/*Triangle tri[8];
		thread t[8];
		for (int i = 0; i < 8; i+=2){
			t[i] = thread(&NaiveSimplifier::simplify2T, this, qTreeLine->subTree[i / 2]->subTree[2 * i % 4],
				qTreeLine->subTree[i / 2]->subTree[2 * i % 4 + 1],
				Rect(qTreeLine->subTree[i / 2]->range.minX,
				(qTreeLine->subTree[i / 2]->range.minX + qTreeLine->subTree[i / 2]->range.maxX) / 2,
				qTreeLine->subTree[i / 2]->range.minY, qTreeLine->subTree[i / 2]->range.maxY),
				tri[i]);
			t[i + 1] = thread(&NaiveSimplifier::simplify2T, this, qTreeLine->subTree[(i + 1) / 2]->subTree[2 * (i + 1) % 4],
				qTreeLine->subTree[(i + 1) / 2]->subTree[2 * (i + 1) % 4 + 1],
				Rect((qTreeLine->subTree[(i + 1) / 2]->range.minX + qTreeLine->subTree[(i + 1) / 2]->range.maxX) / 2,
				qTreeLine->subTree[(i + 1) / 2]->range.maxX,
				qTreeLine->subTree[(i + 1) / 2]->range.minY, qTreeLine->subTree[(i + 1) / 2]->range.maxY),
				tri[i + 1]);
			t[i].join();
			t[i + 1].join();
		}
		for (int i = 0; i < 4; ++i)
			qTreeLine->subTree[i]->size = qTreeLine->subTree[i]->subTree[0]->size + qTreeLine->subTree[i]->subTree[1]->size +
			qTreeLine->subTree[i]->subTree[2]->size + qTreeLine->subTree[i]->subTree[3]->size;


		qTreeLine->size = qTreeLine->subTree[0]->size + qTreeLine->subTree[1]->size +
			qTreeLine->subTree[2]->size + qTreeLine->subTree[3]->size;
		orig_size = qTreeLine->size;
		if (orig_size - qTreeLine->size >= limit)
			return;*/

		/*Triangle tri[16];
		thread t[16];
		for (int i = 0; i < 16; ++i){
			t[i] = thread(&NaiveSimplifier::simplifyT, this, qTreeLine->subTree[i / 4]->subTree[i % 4], qTreeLine->subTree[i / 4]->subTree[i % 4]->range, tri[i]);
			t[i].join();
		}
		for (int i = 0; i < 4; ++i)
			qTreeLine->subTree[i]->size = qTreeLine->subTree[i]->subTree[0]->size + qTreeLine->subTree[i]->subTree[1]->size +
			qTreeLine->subTree[i]->subTree[2]->size + qTreeLine->subTree[i]->subTree[3]->size;
		

		qTreeLine->size = qTreeLine->subTree[0]->size + qTreeLine->subTree[1]->size +
			qTreeLine->subTree[2]->size + qTreeLine->subTree[3]->size;
		orig_size = qTreeLine->size;
		if (orig_size - qTreeLine->size >= limit)
			return;*/
	}
	simplify(limit);
}

/*void NaiveSimplifier::simplify5(int limit){
	Rect box;
	Point *serialBegin, *serialNext;
	double maxX, minX, maxY, minY;
	int last;
	int removed = 1, total_removed = 0;
	while (total_removed < limit && removed != 0){ //repeat the process if more points need to be removed
		for (int i = 0; i < map->lines.size(); ++i){ //for each line
			last = map->lines[i]->points.size();
			if (map->lines[i]->points[last - 1]->leftInd == 0)
				continue; // it is already a segment
			serialBegin = map->lines[i]->points[0];
			if (*(serialBegin) == map->lines[i]->points[last - 1]){ //if the line is a cycle
				serialNext = map->lines[i]->points[serialBegin->rightInd];
				while (serialNext->rightInd < last){
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
		removed = orig_size - qTreeLine->size;
		total_removed += removed;
		orig_size = qTreeLine->size;
	}
}*/