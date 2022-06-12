#pragma once
#include <climits>
#include <vector>
#include "data.h"
#include "parser.h"
using namespace std;

class Via
{
public:
    Via(int x, int y, int w, int h, int netId) : _x(x), _y(y), _w(w), _h(h), _netId(netId) {};
    int getcx()  { return _x; }
    int getcy()  { return _y; }
    int getx1()  { return _x - _w/2; }
    int getx2()  { return _x + _w/2; }
    int gety1()  { return _y - _h/2; }
    int gety2()  { return _y + _h/2; }
    void setcx(int cx)  { _x = cx; }
    void setcy(int cy)  { _y = cy; }
    int getNetId()      { return _netId; }
private:
    int _x;     // center x
    int _y;     // center y
    int _w;
    int _h;
    int _netId; // belongs to net _netId
};

class ViaHandler
{
public:
    ViaHandler(Data& data) : _data(data) {};
    ~ViaHandler() {};

    void placeVia();
    void setData(Parser&, vector<Component*>&);
    void legalize();
    void legalize1();
    void plot();
    vector<Via> getViaVec(){ return _viaVec;}
    int getRelation(Via&, Via&); // horizontal or vertical relation
    vector<pair<int, pair<int, int>>> getViaInfo();

private:
    Data&       _data;
    vector<Via> _viaVec;
    bool**	_grid;
};

