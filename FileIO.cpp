
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
			else if (map->minXs[threadID] > point->x)
				map->minXs[threadID] = point->x;
			if (map->maxYs[threadID] < point->y)
				map->maxYs[threadID] = point->y;
			else if (map->minYs[threadID] > point->y)
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
LineSetM* readLinesM(string filename, const int threadN)
{
	ifstream fin(filename, std::ios::binary | std::ios::ate);
	char* strpos;
	char* firstp;
	int length = fin.tellg();
	length++;
	char* pdata = new char[length];
	fin.seekg(0, ios::beg);
	fin.read(pdata, length);
	pdata[length - 1] = '\0';
	fin.close();

	int quaterLen = length / threadN;

	LineSetM* map = new LineSetM();


	thread t0(readLinesT, map, pdata, pdata + quaterLen, 0);
	thread t1(readLinesT, map, pdata + quaterLen, pdata + 2 * quaterLen, 1);
	thread t2(readLinesT, map, pdata + 2 * quaterLen, pdata + 3 * quaterLen, 2);
	thread t3(readLinesT, map, pdata + 3 * quaterLen, pdata + length - 1, 3);

	t0.join();
	t1.join();
	t2.join();
	t3.join();

	if (map->lines[3][0]->id == map->lines[2][0]->id)
		map->lines[3].clear();
	if (map->lines[2][0]->id == map->lines[1][0]->id)
		map->lines[2].clear();
	if (map->lines[1][0]->id == map->lines[0][0]->id)
		map->lines[1].clear();

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

PointSet* readPoints(string filename)
{
	PointSet* points = new PointSet;
	ifstream fin(filename, std::ios::binary | std::ios::ate);
	string filepoint;
	char* strpos;
	char* firstp;

	vector<char> vfile(fin.tellg());
	fin.seekg(0, ios::beg);
	fin.read(vfile.data(), vfile.size());
	vfile.push_back('\0');
	strpos = vfile.data();

	parseLong(strpos, &firstp);
	firstp += 115;
	points->maxX = points->minX = parseDouble(firstp, &firstp);
	points->maxY = points->minY = parseDouble(++firstp, &firstp);

	clock_t begin = clock();
	while (strpos[0] != '\0'){
		Point* point = new Point;
		parseLong(strpos, &strpos);
		strpos += 115;

		point->x = parseDouble(strpos, &strpos);
		++strpos;
		point->y = parseDouble(strpos, &strpos);
		++strpos;

		if (points->maxX < point->x)
			points->maxX = point->x;
		else if (points->minX > point->x)
			points->minX = point->x;
		if (points->maxY < point->y)
			points->maxY = point->y;
		else if (points->minY > point->y)
			points->minY = point->y;

		strpos += 31;
		points->points.push_back(point);
	}
	cout << "construct points cost: " << clock() - begin << endl;
	return points;
}

void combine(char* output, int& pos, long id){
	if (id == 0){
		output[pos++] = '0';
		return;
	}
	int p = 10000; //limitation, id should not be larger than 10000
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
	while (p >= 1){
		digit = (int)(number / p);
		output[pos++] = char(digit + '0');
		number -= p * digit;
		p /= 10;
	}
	output[pos++] = '.';
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
	for (int k = 0; k < 4; k++)
	{
		vector<Line*> lines = map->lines[k];
		for (int i = 0; i < lines.size(); ++i){
			combine(output, pos, lines[i]->id);
			combine(output, pos, map->gmlLineString, 77);
			combine(output, pos, map->gmlCoordinates, 43);
			ind = 0;
			while (ind < lines[i]->points.size()){
				combine(output, pos, lines[i]->points[ind]->x, ',');
				combine(output, pos, lines[i]->points[ind]->y, ' ');
				ind = lines[i]->points[ind]->rightInd;
			}
			combine(output, pos, map->endCoordinates, 18);
			combine(output, pos, map->endLineString, 17);
			output[pos++] = '\n';
		}
	}

	fwrite(output, sizeof(char), pos, pFile);
	fclose(pFile);
}