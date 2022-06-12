#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<map>
#include<set> // new
using namespace std;

class __Pin
{
public:
	__Pin() {}
	__Pin(int cell, int pin) :_inst(cell), _libPin(pin) {}
	int getInst() { return _inst; }
	int getLibPin() { return _libPin; }
private:
	int _inst; // instance index
	int _libPin; // lib Pin index
};
class __Net
{
public:
	__Net() {}
	void setNumPins(int num) { _numPins = num; _pin.resize(num); };
	void addPin(int index, __Pin p) { _pin[index] = p; };
	__Pin& getPin(int index) { return _pin[index]; }
	int getNumPin() { return _numPins; }
private:
	int _numPins;
	vector<__Pin> _pin; //start from pin[0]
};
class __Row
{
public:
	__Row() {}
	__Row(int startX, int startY, int rowLength, int rowHeight, int repeatCount)
	{
		_startX = startX;
		_startY = startY;
		_rowLength = rowLength;
		_rowHeight = rowHeight;
		_repeatCount = repeatCount;
	}
	int getStartX() { return _startX; }
	int getStartY() { return _startY; }
	int getRowLength() { return _rowLength; }
	int getRowHeight() { return _rowHeight; }
	int getRepeatCount() { return _repeatCount; }

private:
	int _startX;
	int _startY;
	int _rowLength;
	int _rowHeight;
	int _repeatCount;
};
class __Die
{
public:
	__Die() {}
	__Die(int lx, int ly, int ux, int uy)
	{
		_lowerLeftX = lx;
		_lowerLeftY = ly;
		_upperRightX = ux;
		_upperRightY = uy;
		_width = ux - lx;
		_height = uy - ly;
	}
	void setMaxUtil(int u) { _maxUtil = u; }
	void setRow(__Row r) { _row = r; }
	void setTech(int index) { _tech = index; }
	int getMaxUtil() { return _maxUtil; }
	__Row& getRow() { return _row; }
	int getTech() { return _tech; }
	int getWidth() { return _width; }
	int getHeight() { return _height; }

	/* Sam */
	int getLowerLeftX() { return _lowerLeftX; }
	int getLowerLeftY() { return _lowerLeftY; }
	int getUpperRightX() { return _upperRightX; }
	int getUpperRightY() { return _upperRightY; }

private:
	int _lowerLeftX;
	int _lowerLeftY;
	int _upperRightX;
	int _upperRightY;
	int _width;
	int _height;
	int _maxUtil;
	__Row _row;
	int _tech;
};

class __PinLocation
{
public:
	__PinLocation() {}
	int _x;
	int _y;
};
class __LibCell
{
public:
	__LibCell() {}
	void setSize(int x, int y) { _libCellSizeX = x; _libCellSizeY = y; _area = x * y; }
	void setPinCount(int c) { _pinCount = c; _pin.resize(c + 1); }
	void addPin(int index, __PinLocation l) { _pin[index] = l; }
	int getSizeX() { return _libCellSizeX; }
	int getSizeY() { return _libCellSizeY; }
	int getPinCount() { return _pinCount; }
	int getArea() { return _area; }
	vector< __PinLocation> getPin() { return _pin; }

	/* sam */
	__PinLocation getPin(int index) { return _pin[index]; }
private:
	int _area;
	int _libCellSizeX;
	int _libCellSizeY;
	int _pinCount;
	vector<__PinLocation>_pin;	//start from _pin[1]

};
class __Technology
{
public:
	__Technology() {}
	__Technology(const __Technology& other)
	{
		_techName = other._techName;
		_libCellCount = other._libCellCount;
		_libCell.resize(_libCellCount + 1);
		for (int i = 1; i <= _libCellCount; i++)
		{
			_libCell[i] = other._libCell[i];
		}
	}
	void setTechName(string name) { _techName = name; }
	void setLibCellCount(int cellnum) { _libCellCount = cellnum; _libCell.resize(cellnum + 1); }
	void addLibCell(int index, __LibCell cell) { _libCell[index] = cell; }
	__LibCell getLibCell(int index) { return _libCell[index]; }
	string getName() { return _techName; }
	int getLibCellCount() { return _libCellCount; }
private:
	string _techName;
	int _libCellCount;
	vector<__LibCell>_libCell;	//start from _libCell[1]

};
class __Instance
{
public:
	__Instance() {}
	void setLibCellIndex(int index) { _libCellIndex = index; }
	void setTech(int tech, __LibCell libCell) { _tech = tech; _libCell = libCell; }
	void setPos(int x, int y) { _x = x; _y = y; }
	void setWidth(int wid) { _wid = wid; }
	void setHight(int hei) { _hei = hei; }
	int getLibCellIndex() { return _libCellIndex; }
	__LibCell getLibCell() { return _libCell; }
	int getPosX() { return _x; }
	int getPosY() { return _y; }
	int getTech() { return _tech; }
	int getWidth() { return _wid; }
	int getHight() { return _hei; }
	void addNet(int n) 	{ net.insert(n); }// new

	void set__LibCell(__LibCell L) { _libCell = L; }// new

	set <int>& get_net() { return net; }// new

private:
	int _tech;
	int _libCellIndex;
	__LibCell _libCell;
	int _x, _y;
	int _wid, _hei;

	set <int> net; // new
};
