#pragma once
#include "data.h"
typedef long long ll;


class CellLegalizer {
public:
	CellLegalizer(Data &data): _data(data) {}
	void legalize();

private:
	struct Cell {
		Cell() {}
		Cell(__Instance *c, bool die, int pinCount) : inst(c), dieIdx(die), weight(pinCount), x(0), y(0) {}
		__Instance *inst;
		bool dieIdx;
		int weight;
		int x, y;
	};
	struct Cluster {
		Cluster() {}
		Cluster(int xc): x(xc), e(0), w(0), q(0)/*, xt(xc), et(0), wt(0), qt(0)*/ {}
		int x, e, w, q;		// variables for FINAL mode
		//int xt, et, wt, qt;	// variables for TRIAL mode
		vector<Cell> cells;
	};
	struct Row4a { // row for abacus
		Row4a(int n): y(n), cost(0) {}
		int y;
		double cost;
		vector<Cluster> clusters;
		vector<Cluster> bak;
	};

	// for partition
	struct Node {
		Node(int n): id(n), prev(nullptr), next(nullptr) {}
		int					id; 
		Node*				prev;
		Node*				next;
	};
	struct Cell4p {
		Cell4p(__Instance *in, int id, int a1, int a2): inst(in), gain(0), lock(false) { node = new Node(id); area[0] = a1; area[1] = a2; }
		__Instance			*inst;
		Node				*node;
		int					gain;
		int					area[2];
		bool				part;
		bool				lock;
		vector<int>			netList;	
	};
	struct Net4p {
		Net4p() { partCount[0] = 0; partCount[1] = 0; }
		int					partCount[2];
		vector<int>			cellList;
	};


	double rowCost(Row4a *);
	void placeRow(Row4a *, Cell *);
	void collapse(Row4a *);
	void backup(Row4a *);
	void restore(Row4a *);
	void writeback();
	void sort(int left, int right);
	void plotResult(bool, Cell *);
	void plotBoxPLT(ofstream &, int, int, int, int);	
	bool checkResult();

	void partition();
	bool findMaxGain();


	Data &			_data;
	vector<Cell *>	_cellList;
	vector<Row4a *>	_rows[2];
	int				_rowHeight[2];
	int				_rowMinX[2]; 
	int				_rowMinY[2];
	int				_rowMaxX[2];
	int				_rowMaxY[2];
	bool			_curDie;
	bool			_isFinal; // PlaceRow mode

	vector<Cell4p *>_cellArray;
	vector<Net4p *>	_netArray;
	vector<int>		_moveStack;
	map<int, Node *>_bList[2];
	ll				_upperBound[2];
	ll				_partSize[2];
	ll				_dieArea[2];
	int				_maxPinNum;
	int				_cutSize;
	int				_accGain;
	int				_maxAccGain;
	Node *			_maxGainCell;
};