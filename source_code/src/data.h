#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<map>
#include<algorithm>
#include"mymodule.h"
#include "segment.h" //new

using namespace std;

class Data
{
public:
	Data(string fileName);
	void output(string fileName, vector<pair<int, pair<int, int> > > viaInfo);
	int getINSTANCECount() { return  _instanceCount; }
	int getNETCount() { return _netCount; }
	int getTerminalSizeX() { return _terminalSizeX; }
	int getTerminalSizeY() { return _terminalSizeY; }
	int getTerminalSpacing() { return _terminalSpacing; }
	int getTechnologyCount() { return _technologyCount; }
	__Die& getDIE(int index) { return _dies[index]; }
	__Instance& getINSTANCE(int index) { return _instances[index]; }
	__Technology& getTECHNOLOGY(int index) { return _technologies[index]; }
	__Net& getNET(int index) { return _nets[index]; }

	void DP(int iteration); // new
	void BB(segment& seg, int step_size, vector < pair<int, int> >& free_space
		, long long int& best_WL, long long int current_WL, vector <int>& best_pos, int current_ins); // new
	int calcWL(int n) // new
	{
		int x1 = _dies[0].getUpperRightX();
		int x2 = _dies[0].getLowerLeftX();
		int y1 = _dies[0].getUpperRightY();
		int y2 = _dies[0].getLowerLeftY();
		for (int i = 0; i < _nets[n].getNumPin(); i++)
		{
			int index = _nets[n].getPin(i).getInst();
			int x = _instances[index].getPosX();
			int y = _instances[index].getPosY();

			int pin_index = _nets[n].getPin(i).getLibPin();
			int pin_x = _technologies[_dies[_instances[index].getTech()].getTech()].getLibCell(_instances[index].getLibCellIndex())
				.getPin()[pin_index]._x;
			int pin_y = _technologies[_dies[_instances[index].getTech()].getTech()].getLibCell(_instances[index].getLibCellIndex())
				.getPin()[pin_index]._y;

			pin_x += x;
			pin_y += y;

			if (pin_x < x1)
				x1 = pin_x;
			if (pin_x > x2)
				x2 = pin_x;

			if (pin_y < y1)
				y1 = pin_y;
			if (pin_y > y2)
				y2 = pin_y;
		}
		//cout << x2 << ' ' << x1 << ' ' << y2 << ' ' << y2 << '\n';

		return (x2 - x1) + (y2 - y1);
	}
	int calc_partWL(int n, vector <int>& pass) // new
	{
		int x1 = _dies[0].getUpperRightX();
		int x2 = _dies[0].getLowerLeftX();
		int y1 = _dies[0].getUpperRightY();
		int y2 = _dies[0].getLowerLeftY();
		for (int i = 0; i < _nets[n].getNumPin(); i++)
		{
			int index = _nets[n].getPin(i).getInst();

			auto it = find(pass.begin(), pass.end(), index);
			if (it != pass.end())
				continue;

			int x = _instances[index].getPosX();
			int y = _instances[index].getPosY();

			int pin_index = _nets[n].getPin(i).getLibPin();
			int pin_x = _technologies[_dies[_instances[index].getTech()].getTech()].getLibCell(_instances[index].getLibCellIndex())
				.getPin()[pin_index]._x;
			int pin_y = _technologies[_dies[_instances[index].getTech()].getTech()].getLibCell(_instances[index].getLibCellIndex())
				.getPin()[pin_index]._y;

			pin_x += x;
			pin_y += y;

			if (pin_x < x1)
				x1 = pin_x;
			if (pin_x > x2)
				x2 = pin_x;

			if (pin_y < y1)
				y1 = pin_y;
			if (pin_y > y2)
				y2 = pin_y;
		}

		return (x2 - x1) + (y2 - y1);
	}

private:
	int _instanceCount;
	int _netCount;
	int _technologyCount;
	int _terminalSizeX;
	int _terminalSizeY;
	int _terminalSpacing;
	vector<__Instance>_instances; // start with _instances[1]
	vector<__Net>_nets;	// start with _nets[1]
	vector<__Die> _dies;	// 0: TopDIE 1: BottomDIE
	vector<__Technology>_technologies;

};
