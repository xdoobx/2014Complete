
#include "stdafx.h"
#include "FileIO.h"

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

LineSet* readLines(string filename)
{
	LineSet* map = new LineSet;
	ifstream fin(filename, std::ios::binary | std::ios::ate);
	char* strpos;
	char* firstp;

	vector<char> vfile(fin.tellg());
	fin.seekg(0, ios::beg);
	fin.read(vfile.data(), vfile.size());
	vfile.push_back('\0');
	strpos = vfile.data();

	parseLong(strpos, &firstp);
	firstp += 120;
	map->maxX = map->minX = parseDouble(firstp, &firstp);
	map->maxY = map->minY = parseDouble(++firstp, &firstp);

	//clock_t begin = clock();
	while (strpos[0] != '\0'){
		Line* line = new Line;
		line->id = parseLong(strpos, &strpos);
		strpos += 120;
		while (strpos[0] != '<' && strpos[0] != '/'){
			Point* point = new Point;
			point->x = parseDouble(strpos, &strpos);
			++strpos;
			point->y = parseDouble(strpos, &strpos);
			++strpos;
			if (map->maxX < point->x)
				map->maxX = point->x;
			else if (map->minX > point->x)
				map->minX = point->x;
			if (map->maxY < point->y)
				map->maxY = point->y;
			else if (map->minY > point->y)
				map->minY = point->y;
			point->pointInd = line->points.size();
			point->lineInd = map->lines.size();
			point->leftInd = line->points.size() - 1;
			point->rightInd = line->points.size() + 1;
			line->points.push_back(point);
			++line->kept;
		}
		if (*line->points[0] == line->points[line->points.size() - 1])
			line->cycle = true;
		else
			line->cycle = false;
		strpos += 36;
		map->lines.push_back(line);
	}
	//cout << "construct line cost: " << clock() - begin << endl;
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
	return points;
}

void combine(char* output, int& pos, long id){
	if (id == 0){
		output[pos++] = '0';
		return;
	}
	int p = 10000; //limitation, id should not be larger than 10000
	int digit = 0;
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

void writeLines(LineSet* map, string filename, int length)
{
	FILE * pFile;
	fopen_s(&pFile, filename.c_str(), "wb");
	char* output = new char[length];
	int pos = 0;
	int ind = 0;
	for (int i = 0; i < map->lines.size(); ++i){
		combine(output, pos, map->lines[i]->id);
		combine(output, pos, map->gmlLineString, 77);
		combine(output, pos, map->gmlCoordinates, 43);
		ind = 0;
		while(ind < map->lines[i]->points.size()){
			combine(output, pos, map->lines[i]->points[ind]->x, ',');
			combine(output, pos, map->lines[i]->points[ind]->y, ' ');
			ind = map->lines[i]->points[ind]->rightInd;
		}
		combine(output, pos, map->endCoordinates, 18);
		combine(output, pos, map->endLineString, 17);
		output[pos++] = '\n';
	}
	fwrite(output, sizeof(char), pos, pFile);
	fclose(pFile);
}