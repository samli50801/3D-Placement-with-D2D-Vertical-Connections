#pragma once

#include"mymodule.h"

#include<vector>

class segment
{
public:
	segment() {}
	segment(int w, int x1, int x2, int y)
	{
		width = w;
		this->y = y;
		this->x1 = x1;
		this->x2 = x2;
		instance.resize(0);
	}

	void set_x1(int x) { x1 = x; }
	void set_x2(int x) { x2 = x; }

	void add_ins(int id) { instance.push_back(id); }
	void pop_ins() { instance.pop_back(); }
	void add_net(int id) { net.insert(id); }

	int get_width() { return width; }
	int get_y() { return y; }
	int get_x1() { return x1; }
	int get_x2() { return x2; }
	vector <int>& get_ins() { return instance; }
	set <int>& get_net() { return net; }

private:
	int width;
	int y;
	int x1;
	int x2;

	vector <int> instance;
	set <int> net;
};
