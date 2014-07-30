
#include "stdafx.h"
#include "FileIO.h"
#include <thread>

/*Thanks to 6502 on stackoverflow*/
double parseDouble(char* p, char** pos)
{
	int s = 1;

	if (*p == '-') {
		s = -1; p++;
	}

	double acc = 0;
	while (*p >= '0' && *p <= '9')
		acc = acc * 10 + *p++ - '0';

	if (*p == '.') {
		double k = 0.1;
		p++;
		while (*p >= '0' && *p <= '9') {
			acc += (*p++ - '0') * k;
			k *= 0.1;
		}
	}
	*pos = p;
	return s * acc;
}

long parseLong(char* p, char** pos){
	long acc = 0;
	while (*p != ':')
		acc = acc * 10 + *p++ - '0';
	*pos = p;
	return acc;
}


//
char* findStart(char* p)
{
	//stop at the ":<"
	while (*p != ':' || (*(p + 1) != '<'))
	{
		p++;
	}

	p--;//reverse from : to a digit

	//rev back before the ID
	while (*p <= '9' && *p >= '0')
	{
		p--;
	}

	p++;//start at the ID.

	return p;
}

//before entering the lineSet has been constructed.
void readLinesT(LineSetM* map, char *pData, char* endMark, int threadID)
{
	char* strpos = findStart(pData);

	map->maxXs[threadID] = map->maxXs[threadID] = -1 << 30;
	map->minXs[threadID] = map->minYs[threadID] = 1 << 30;

	map->num_points[threadID] = 0;

	while (strpos < endMark){
		Line* line = new Line;
		line->id = parseLong(strpos, &strpos);
		strpos += 120;
		while (strpos[0] != '<'){
			Point* point = new Point;
			point->x = parseDouble(strpos, &strpos);
			++strpos;
			point->y = parseDouble(strpos, &strpos);
			++strpos;
			if (map->maxXs[threadID] < point->x)
				map->maxXs[threadID] = point->x;
			if (map->minXs[threadID] > point->x)
				map->minXs[threadID] = point->x;
			if (map->maxYs[threadID] < point->y)
				map->maxYs[threadID] = point->y;
			if (map->minYs[threadID] > point->y)
				map->minYs[threadID] = point->y;
			point->pointInd = line->points.size();
			point->lineInd = map->lines[threadID].size();
			point->leftInd = line->points.size() - 1;
			point->rightInd = line->points.size() + 1;
			line->points.push_back(point);
			++line->kept;
			++map->num_points[threadID];
		}
		if (*line->points[0] == line->points[line->points.size() - 1])
			line->cycle = true;
		else
			line->cycle = false;
		line->share = false;
		strpos += 36;
		map->lines[threadID].push_back(line);
	}
}

//threadN is the maximal number of threads.
LineSetM* readLinesM(string filename)
{
	ifstream fin(filename, std::ios::binary | std::ios::ate);
	int length = fin.tellg();
	length++;
	char* pdata = new char[length];
	fin.seekg(0, ios::beg);
	fin.read(pdata, length);
	pdata[length - 1] = '\0';
	fin.close();

	LineSetM* map = new LineSetM();

	int threadN = map->threadN;

	int quaterLen = length / threadN;

	thread* t = new thread[threadN];
	for (int i = 0; i < threadN-1; ++i)
		t[i] = thread(readLinesT, map, pdata + i * quaterLen, pdata + (i + 1) * quaterLen, i);
	t[threadN - 1] = thread(readLinesT, map, pdata + (threadN - 1) * quaterLen, pdata + length - 1, threadN - 1);
	for (int i = 0; i < threadN; ++i)
		t[i].join();

	for (int i = threadN - 1; i > 0; --i){
		if (map->lines[i][0]->id == map->lines[i - 1][0]->id)
			map->lines[i].clear();
	}

	map->minx = map->minXs[0];
	map->maxx = map->maxXs[0];
	map->miny = map->minYs[0];
	map->maxy = map->maxYs[0];

	for (int i = 1; i < threadN; i++)
	{
		map->minx = (map->minXs[i] < map->minx) ? map->minXs[i] : map->minx;
		map->maxx = (map->maxXs[i] > map->maxx) ? map->maxXs[i] : map->maxx;

		map->miny = (map->minYs[i] < map->miny) ? map->minYs[i] : map->miny;
		map->maxy = (map->maxYs[i] > map->maxy) ? map->maxYs[i] : map->maxy;
	}

	return map;
}

void readPointsT(PointSetM* points, char *pData, char *endMark, int threadID)
{
	char* strpos = findStart(pData);

	points->maxXs[threadID] = points->maxXs[threadID] = -1 << 30;
	points->minXs[threadID] = points->minYs[threadID] = 1 << 30;

	while (strpos < endMark){
		Point* point = new Point;
		parseLong(strpos, &strpos);
		strpos += 115;

		point->x = parseDouble(strpos, &strpos);
		++strpos;
		point->y = parseDouble(strpos, &strpos);
		++strpos;

		if (points->maxXs[threadID] < point->x)
			points->maxXs[threadID] = point->x;
		if (points->minXs[threadID] > point->x)
			points->minXs[threadID] = point->x;
		if (points->maxYs[threadID] < point->y)
			points->maxYs[threadID] = point->y;
		if (points->minYs[threadID] > point->y)
			points->minYs[threadID] = point->y;

		strpos += 31;
		points->point[threadID].push_back(point);
	}
}

PointSetM* readPointsM(string filename)
{
	PointSetM* points = new PointSetM;
	ifstream fin(filename, std::ios::binary | std::ios::ate);

	int length = fin.tellg();
	length++;
	char* pdata = new char[length];
	fin.seekg(0, ios::beg);
	fin.read(pdata, length);
	pdata[length - 1] = '\0';
	fin.close();

	int threadN = points->threadN;

	int quaterLen = length / threadN;

	thread* t = new thread[threadN];
	for (int i = 0; i < threadN - 1; ++i)
		t[i] = thread(readPointsT, points, pdata + i * quaterLen, pdata + (i + 1) * quaterLen, i);
	t[threadN - 1] = thread(readPointsT, points, pdata + (threadN - 1) * quaterLen, pdata + length - 1, threadN - 1);
	for (int i = 0; i < threadN; ++i)
		t[i].join();

	for (int i = threadN - 1; i > 0; --i){
		if (*points->point[i][0] == points->point[i - 1][0])
			points->point[i].clear();
	}

	points->minX = points->minXs[0];
	points->maxX = points->maxXs[0];
	points->minY = points->minYs[0];
	points->maxY = points->maxYs[0];

	for (int i = 1; i < threadN; i++)
	{
		points->minX = (points->minXs[i] < points->minX) ? points->minXs[i] : points->minX;
		points->maxX = (points->maxXs[i] > points->maxX) ? points->maxXs[i] : points->maxX;

		points->minY = (points->minYs[i] < points->minY) ? points->minYs[i] : points->minY;
		points->maxY = (points->maxYs[i] > points->maxY) ? points->maxYs[i] : points->maxY;
	}

	return points;
}

void combine(char* output, int& pos, long id){
	if (id == 0){
		output[pos++] = '0';
		return;
	}
	int p = 10000; //limitation, id should not be larger than 100000
	int digit;
	while (id / p < 1)
		p /= 10;
	while (p > 0){
		digit = id / p;
		output[pos++] = char(digit + '0');
		id -= digit * p;
		p /= 10;
	}
}

void combine(char* output, int& pos, double number, char sep){
	double p = 1e8; // limitation, number should not be larger than 1e8
	int digit;
	if (number < 0){
		output[pos++] = '-';
		number *= -1;
	}
	else if (number == 0){
		output[pos++] = '0';
		output[pos++] = sep;
		return;
	}
	while (number / p < 1)
		p /= 10;
	if (p < 1)
		output[pos++] = '0';
	while (p >= 1){
		digit = (int)(number / p);
		output[pos++] = char(digit + '0');
		number -= p * digit;
		p /= 10;
	}
	output[pos++] = '.';
	if (p < 0.0001)
		output[pos++] = '0';
	while (p >= 0.0001){
		digit = (int)(number / p);
		output[pos++] = char(digit + '0');
		number -= p * digit;
		p /= 10;
	}
	output[pos++] = sep;
}

void combine(char* output, int& pos, const char* str, int length){
	for (int i = 0; i < length; ++i)
		output[pos + i] = str[i];
	pos += length;
}

void writeLinesM(LineSetM* map, string filename, int length)
{
	FILE * pFile;
	fopen_s(&pFile, filename.c_str(), "wb");
	char* output = new char[length];
	int pos = 0;
	int ind = 0;
	for (int k = 0; k < map->threadN; k++)
	{
		for (int i = 0; i < map->lines[k].size(); ++i){
			combine(output, pos, map->lines[k][i]->id);
			combine(output, pos, map->gmlLineString, 77);
			combine(output, pos, map->gmlCoordinates, 43);
			ind = 0;
			while (ind < map->lines[k][i]->points.size()){
				combine(output, pos, map->lines[k][i]->points[ind]->x, ',');
				combine(output, pos, map->lines[k][i]->points[ind]->y, ' ');
				ind = map->lines[k][i]->points[ind]->rightInd;
			}
			combine(output, pos, map->endCoordinates, 18);
			combine(output, pos, map->endLineString, 17);
			output[pos++] = '\n';
		}
	}

	fwrite(output, sizeof(char), pos, pFile);
	fclose(pFile);
}