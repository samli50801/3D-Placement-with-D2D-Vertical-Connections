#define _GLIBCXX_USE_CXX11_ABI 0
#ifndef GLOBALPLACER_H
#define GLOBALPLACER_H

#include "data.h"
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <string>

class GlobalPlacer 
{
public:
    
    GlobalPlacer(Data& datab);
	void place();
    void plotPlacementResult(string name, vector<double> x);
    void plotBoxPLT(ofstream& stream, double x1, double y1, double x2, double y2);

private:
    Data& _data;
};

#endif // GLOBALPLACER_H
