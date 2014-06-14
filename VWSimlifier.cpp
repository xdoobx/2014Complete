
#include "stdafx.h"
#include "VWSimplifier.h"
#include <algorithm>

VWSimplifier::VWSimplifier(char* lineFile, char* pointFile) : NaiveSimplifier(lineFile, pointFile){
	double halfW = qTreeLine->subTree[0]->range.maxX;
	double halfH = qTreeLine->subTree[0]->range.maxY;
	Triangle* tri = NULL;
	Triangle* last_tri;
	for (int i = 0; i < map->lines.size(); ++i){
		if (map->lines[i]->points.size() > 2){ //first triangle
			tri = new Triangle;
			tri->p[0] = map->lines[i]->points[0];
			tri->p[1] = map->lines[i]->points[1];
			tri->p[2] = map->lines[i]->points[2];
			tri->sort();
			tri->calArea();
			tri->pre = NULL;

			if (tri->p[1]->x < halfW){
				if (tri->p[1]->y < halfH)
					tri_area[0].push_back(tri);
				else
					tri_area[1].push_back(tri);
			}
			else{
				if (tri->p[1]->y < halfH)
					tri_area[3].push_back(tri);
				else
					tri_area[2].push_back(tri);
			}
		}
		for (int j = 2; j < map->lines[i]->points.size() - 1; ++j){
			last_tri = tri;
			tri = new Triangle;
			tri->p[0] = map->lines[i]->points[j - 1];
			tri->p[1] = map->lines[i]->points[j];
			tri->p[2] = map->lines[i]->points[j + 1];
			tri->sort();
			tri->calArea();
			tri->pre = last_tri;
			last_tri->next = tri;
			if (tri->p[1]->x < halfW){
				if (tri->p[1]->y < halfH)
					tri_area[0].push_back(tri);
				else
					tri_area[1].push_back(tri);
			}
			else{
				if (tri->p[1]->y < halfH)
					tri_area[3].push_back(tri);
				else
					tri_area[2].push_back(tri);
			}
		}
		tri->next = NULL;
	}
}

//function to judge which triangle is bigger
inline bool comp(Triangle* tri1, Triangle* tri2){
	return tri1->area > tri2->area;
}

inline void VWSimplifier::heap_update(int heap, int index){
	Triangle* temp;
	Triangle* to_swap;
	
	//upward swap
	if (tri_area[heap][index]->area < tri_area[heap][index / 2]->area){
		while (tri_area[heap][index]->area < tri_area[heap][index / 2]->area){
			temp = tri_area[heap][index];
			tri_area[heap][index] = tri_area[heap][index / 2];
			tri_area[heap][index / 2] = temp;
			tri_area[heap][index / 2]->ind = tri_area[heap][index]->ind;
			tri_area[heap][index]->ind = index;
			index /= 2;
		}
		return;
	}

	//downward swap
	if (index * 2 + 2 < heap_size[heap]){
		if (tri_area[heap][index * 2 + 1]->area > tri_area[heap][index * 2 + 2]->area)
			to_swap = tri_area[heap][index * 2 + 2];
		else
			to_swap = tri_area[heap][index * 2 + 1];
	}
	else if (index * 2 + 1 < heap_size[heap])
		to_swap = tri_area[heap][index * 2 + 1];
	else
		return;

	if (tri_area[heap][index]->area > to_swap->area){
		while (tri_area[heap][index]->area > to_swap->area){
			temp = tri_area[heap][index];
			tri_area[heap][index] = to_swap;
			tri_area[heap][to_swap->ind] = temp;
			tri_area[heap][to_swap->ind]->ind = to_swap->ind;
			tri_area[heap][index]->ind = index;
			index = temp->ind;
			if (index * 2 + 2 < heap_size[heap]){
				if (tri_area[heap][index * 2 + 1]->area > tri_area[heap][index * 2 + 2]->area)
					to_swap = tri_area[heap][index * 2 + 2];
				else
					to_swap = tri_area[heap][index * 2 + 1];
			}
			else if (index * 2 + 1 < heap_size[heap])
				to_swap = tri_area[heap][index * 2 + 1];
			else
				return;
		}
	}
}

inline void VWSimplifier::heap_pop(int heap){
	Triangle* temp = tri_area[heap][0];
	Triangle* to_swap;
	int index = 0;
	tri_area[heap][0] = tri_area[heap][heap_size[heap]];
	tri_area[heap][heap_size[heap]] = temp;
	tri_area[heap][heap_size[heap]]->ind = tri_area[heap][0]->ind;
	tri_area[heap][0]->ind = 0;

	if (index * 2 + 2 < heap_size[heap]){
		if (tri_area[heap][index * 2 + 1]->area > tri_area[heap][index * 2 + 2]->area)
			to_swap = tri_area[heap][index * 2 + 2];
		else
			to_swap = tri_area[heap][index * 2 + 1];
	}
	else if (index * 2 + 1 < heap_size[heap])
		to_swap = tri_area[heap][index * 2 + 1];
	else
		return;

	if (tri_area[heap][index]->area > to_swap->area){
		while (tri_area[heap][index]->area > to_swap->area){
			temp = tri_area[heap][index];
			tri_area[heap][index] = to_swap;
			tri_area[heap][to_swap->ind] = temp;
			tri_area[heap][to_swap->ind]->ind = to_swap->ind;
			tri_area[heap][index]->ind = index;
			index = temp->ind;
			if (index * 2 + 2 < heap_size[heap]){
				if (tri_area[heap][index * 2 + 1]->area > tri_area[heap][index * 2 + 2]->area)
					to_swap = tri_area[heap][index * 2 + 2];
				else
					to_swap = tri_area[heap][index * 2 + 1];
			}
			else if (index * 2 + 1 < heap_size[heap])
				to_swap = tri_area[heap][index * 2 + 1];
			else
				return;
		}
	}
}

void VWSimplifier::simplifyT(int heap, int limit){ // in each thread
	double halfW = qTreeLine->subTree[0]->range.maxX;
	double halfH = qTreeLine->subTree[0]->range.maxY;
	heap_size[heap] = tri_area[heap].size() - 1;
	Triangle* back = NULL; // record triangle to be removed

	make_heap(tri_area[heap].begin(), tri_area[heap].end(), comp);
	for (int i = 0; i <= heap_size[heap]; ++i){
		tri_area[heap][i]->ind = i;
	}

	for (int i = 0; i < limit; ++i){
		heap_pop(heap);
		back = tri_area[heap][heap_size[heap]];
		--heap_size[heap];
		if (back->maxX > qTreeLine->subTree[heap]->range.maxX ||
			back->maxY > qTreeLine->subTree[heap]->range.maxY ||
			back->minX <= qTreeLine->subTree[heap]->range.minX ||
			back->minY <= qTreeLine->subTree[heap]->range.minY ||
			!removeS(*back)){
			--i; // not counted as removed
			if (back->pre != NULL)
				back->pre->next = NULL;
			if (back->next != NULL)
				back->next->pre = NULL;
		}
		else{ // update two neighbour triangles
			if (back->pre != NULL){
				back->pre->p[2] = map->lines[back->pre->p[1]->lineInd]->points[back->pre->p[1]->rightInd];
				back->pre->sort();
				back->pre->calArea();
				back->pre->next = back->next;
				//TO BE OPTIMIZED 
				if (back->pre->p[1]->x < halfW){
					if (back->pre->p[1]->y < halfH)
						heap_update(0, back->pre->ind);
					else
						heap_update(1, back->pre->ind);
				}
				else{
					if (back->pre->p[1]->y < halfH)
						heap_update(3, back->pre->ind);
					else
						heap_update(2, back->pre->ind);
				}
				//END TO BE OPTIMIZED
			}
			if (back->next != NULL){
				back->next->p[0] = map->lines[back->next->p[1]->lineInd]->points[back->next->p[1]->leftInd];
				back->next->sort();
				back->next->calArea();
				back->next->pre = back->pre;
				if (back->next->p[1]->x < halfW){
					if (back->next->p[1]->y < halfH)
						heap_update(0, back->next->ind);
					else
						heap_update(1, back->next->ind);
				}
				else{
					if (back->next->p[1]->y < halfH)
						heap_update(3, back->next->ind);
					else
						heap_update(2, back->next->ind);
				}
			}
		}
		//tri_area[heap].pop_back();
		if (heap_size[heap] == 0)
			break;
	}
}

void VWSimplifier::simplifyMT(int limit){
	if (qTreeLine->subTree[0] != NULL){
		int limits[4];
		limits[0] = limit*qTreeLine->subTree[0]->size / qTreeLine->size;
		limits[1] = limit*qTreeLine->subTree[1]->size / qTreeLine->size;
		limits[2] = limit*qTreeLine->subTree[2]->size / qTreeLine->size;
		limits[3] = limit*qTreeLine->subTree[3]->size / qTreeLine->size;

		thread t0(&VWSimplifier::simplifyT, this, 0, limits[0] + 1);
		thread t1(&VWSimplifier::simplifyT, this, 1, limits[1] + 1);
		thread t2(&VWSimplifier::simplifyT, this, 2, limits[2] + 1);
		thread t3(&VWSimplifier::simplifyT, this, 3, limits[3] + 1);
		t0.join();
		t1.join();
		t2.join();
		t3.join();
		// the root size was messed up by multi-thread
		qTreeLine->size = qTreeLine->subTree[0]->size + qTreeLine->subTree[1]->size +
			qTreeLine->subTree[2]->size + qTreeLine->subTree[3]->size;
		if (orig_size - qTreeLine->size >= limit)
			return;
	}
	simplify(limit);
}