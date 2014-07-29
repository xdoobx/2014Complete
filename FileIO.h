#ifndef _FILEIO_H_
#define _FILEIO_H_

#include "GeoUtility.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <limits>
#include <time.h>

extern void readPointsT(PointSetM* points, char *pData, char *endMark, int threadID);
extern PointSetM* readPointsM(string filename);
extern void readLinesT(LineSetM* map, char *pData, char* endMark, int threadID);
extern LineSetM* readLinesM(string filename);

extern void writeLinesM(LineSetM* map, string filename, int length);

#endif