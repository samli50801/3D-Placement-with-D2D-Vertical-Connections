#include "CellLegalizer.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <climits>
#define REP(i, a, b) for (int i = a; i < b; i++)
#define rep(i, n) REP(i, 0, n)

void CellLegalizer::legalize() {
	// FM, aims to get the minimal cut (i.e. number of vias) while the slack of both dies is as balanced as possible.
	// by slack I mean MaxUtil * die Size - total Cell Area on this die.
	partition();

	// psudo global placement, all cells are placed randomly .
	rep(i, _data.getINSTANCECount()) {
		__Instance &curCell = _data.getINSTANCE(i + 1);
		__Die &die = _data.getDIE(curCell.getTech());
		int xmax(die.getWidth()  - curCell.getLibCell().getSizeX());
		int ymax(die.getHeight() - curCell.getLibCell().getSizeY());
		// *********************************************************
		// * Comment out this line ↓ if global placement is added. *
		     //curCell.setPos(rand() % xmax, rand() % ymax);
		// * Comment out this line ↑ if global placement is added. *
		// *********************************************************
		// construct my data structure for abacus
		_cellList.push_back(new Cell(&curCell, curCell.getTech(), curCell.getLibCell().getPinCount()));
	}

	// construct each row
	rep(i, 2) {
		_rowHeight[i] = _data.getDIE(i).getRow().getRowHeight();
		_rowMinX[i] = _data.getDIE(i).getRow().getStartX();
		_rowMinY[i] = _data.getDIE(i).getRow().getStartY();
		_rowMaxX[i] = _data.getDIE(i).getRow().getRowLength() + _rowMinX[i];
		_rowMaxY[i] = _data.getDIE(i).getRow().getRepeatCount() * _rowHeight[i] + _rowMinY[i];		
		for (int j(0), y = _data.getDIE(i).getRow().getStartY(); j < _data.getDIE(i).getRow().getRepeatCount(); j++, y += _rowHeight[i]) 
			_rows[i].push_back(new Row4a(y));
	}

	// abacus
	sort(0, _cellList.size() - 1);
	for (Cell *curCell: _cellList) {
		double cbest = numeric_limits<double>::max();
		
		_curDie = curCell->dieIdx;

		int rbest = -1;
		int startRow = (double)(curCell->inst->getPosY() - _rows[_curDie][0]->y) / _rowHeight[_curDie] + .5;

		// search upward
		REP(y, startRow, _rows[_curDie].size()) {
			if (_rows[_curDie][y]->cost + abs(_rows[_curDie][y]->y - curCell->inst->getPosY()) > cbest) 
				break;
			backup(_rows[_curDie][y]);
			placeRow(_rows[_curDie][y], curCell);
			double cost = rowCost(_rows[_curDie][y]);
			if (cost < cbest) {
				cbest = cost;
				rbest = y;
			}
			restore(_rows[_curDie][y]);
		}
		// search downward
		for (startRow--; startRow >= 0 && _rows[_curDie][startRow]->cost + abs(_rows[_curDie][startRow]->y - curCell->inst->getPosY()) <= cbest; startRow--) {
			backup(_rows[_curDie][startRow]);
			placeRow(_rows[_curDie][startRow], curCell);
			double cost = rowCost(_rows[_curDie][startRow]);
			if (cost < cbest) {
				cbest = cost;
				rbest = startRow;
			}
			restore(_rows[_curDie][startRow]);
		}

		if (rbest == -1) {
			cerr << "-1, Legalization failed ..." << endl;
			writeback();
			//plotResult(0, curCell);
			//plotResult(1, curCell);

			// clean the memory 
			for (Cell *cell : _cellList)
				delete cell;
			rep(i, 2) for (Row4a *row : _rows[i])
				delete row;
			return;
		}

		placeRow(_rows[_curDie][rbest], curCell);
		rowCost(_rows[_curDie][rbest]);
	}

	writeback();
	//plotResult(0, nullptr);
	//plotResult(1, nullptr);
	cout << "Legalization " << (checkResult()? "success!": "failed...") << endl;

	// clean the memory 
	for (Cell *cell: _cellList)
		delete cell;
	rep(i, 2) for (Row4a *row: _rows[i])
		delete row;
}



/*********************/
/* Private Functions */
/*********************/

void CellLegalizer::partition() {
	// construct my data structures
	rep(i, 2) _dieArea[i] = (ll)_data.getDIE(i).getWidth() * _data.getDIE(i).getHeight();
	rep(i, 2) { _partSize[i] = 0; _upperBound[i] = _dieArea[i] * (_data.getDIE(i).getMaxUtil() / 100.); }	
	REP(i, 1, _data.getINSTANCECount() + 1) {
		int lCellIdx = _data.getINSTANCE(i).getLibCellIndex();
		int a1 = _data.getTECHNOLOGY(_data.getDIE(0).getTech()).getLibCell(lCellIdx).getArea();
		int a2 = _data.getTECHNOLOGY(_data.getDIE(1).getTech()).getLibCell(lCellIdx).getArea();
		_cellArray.push_back(new Cell4p(&_data.getINSTANCE(i), i - 1, a1, a2));
	}

	// initial partition
	bool tmpPart = 0;
	for (Cell4p *cell : _cellArray) {
		cell->part = tmpPart;
		_partSize[tmpPart] += cell->area[tmpPart];
		if (_upperBound[tmpPart] - _partSize[tmpPart] < _upperBound[!tmpPart] - _partSize[!tmpPart]) {
			tmpPart = !tmpPart;
		}
	} 

	// for each net get its part count 
	REP(i, 1, _data.getNETCount() + 1) {
		_netArray.push_back(new Net4p());
		rep(j, _data.getNET(i).getNumPin()) {
			_netArray.back()->cellList.push_back(_data.getNET(i).getPin(j).getInst() - 1);
			_cellArray[_netArray.back()->cellList.back()]->netList.push_back(i - 1);
			_netArray.back()->partCount[_cellArray[_netArray.back()->cellList.back()]->part]++;
		}
	}

	

	// evaluate the initial cut size
	_cutSize = 0;
	for (Net4p *net : _netArray) {
		if (net->partCount[0] && net->partCount[1])
			_cutSize++;
	} 

	// get the max p(i)
	_maxPinNum = 0;
	for (Cell4p *c : _cellArray) {
		if (c->netList.size() > _maxPinNum)
			_maxPinNum = c->netList.size();
	}

	// algorithm starts
	do {
		// initialize the bucket list
		for (int i = _maxPinNum; i >= -_maxPinNum; i--) {
			_bList[0][i] = nullptr;
			_bList[1][i] = nullptr;
		}
		for (Cell4p *cell: _cellArray) {
			// evaluate the initial gain
			cell->gain = 0;
			for (int nIdx : cell->netList) {
				//  F(n) = 1, g(i) + 1
				if (_netArray[nIdx]->partCount[cell->part] == 1)
					cell->gain++;
				// T(n) = 0, g(i) - 1
				if (_netArray[nIdx]->partCount[!cell->part] == 0)
					cell->gain--;
			}

			cell->lock = false;

			// setup the bucket list
			cell->node->next = _bList[cell->part][cell->gain];
			cell->node->prev = nullptr;
			if (cell->node->next)
				cell->node->next->prev = cell->node;
			_bList[cell->part][cell->gain] = cell->node;
		}

		// FM pass
		_accGain = 0, _maxAccGain = 0;
		int step, k(INT_MAX), diff(0);
		for (step = 0; findMaxGain(); step++) {
			Cell4p *curCell = _cellArray[_maxGainCell->id];			

			_moveStack.push_back(_maxGainCell->id);
			_accGain += curCell->gain;
			if (_accGain > _maxAccGain) {
				_maxAccGain = _accGain;
				k = step;
				diff = abs(_upperBound[0] - _partSize[0] - _upperBound[1] + _partSize[1]);
			}
			else if (_accGain == _maxAccGain) {
				int curDiff = abs(_upperBound[0] - _partSize[0] - _upperBound[1] + _partSize[1]);
				if (curDiff < diff) {
					diff = curDiff;
					k = step;
				}
			}

			// updated cells and their initial gain
			map<Cell4p *, int> updatedCell;

			// Update_Gain() on unit3 ppt page 28
			bool fromPart = curCell->part;
			_partSize[fromPart]	 -= curCell->area[fromPart];
			_partSize[!fromPart] += curCell->area[!fromPart];
			curCell->part = !curCell->part;
			curCell->lock = true;
			for (int netidx: curCell->netList) {
				if (_netArray[netidx]->partCount[!fromPart] == 0) {
					for (int cellidx: _netArray[netidx]->cellList) {
						if (!_cellArray[cellidx]->lock) {
							if (updatedCell.count(_cellArray[cellidx]) == 0)
								updatedCell[_cellArray[cellidx]] = _cellArray[cellidx]->gain;
							_cellArray[cellidx]->gain++;
						}
					}
				}
				else if (_netArray[netidx]->partCount[!fromPart] == 1) {
					for (int cellidx: _netArray[netidx]->cellList) {
						if (!_cellArray[cellidx]->lock && _cellArray[cellidx]->part != fromPart) {
							if (updatedCell.count(_cellArray[cellidx]) == 0)
								updatedCell[_cellArray[cellidx]] = _cellArray[cellidx]->gain;
							_cellArray[cellidx]->gain--;
							break;
						}
					}
				}

				_netArray[netidx]->partCount[fromPart]--;
				_netArray[netidx]->partCount[!fromPart]++;

				if (_netArray[netidx]->partCount[fromPart] == 0) {
					for (int cellidx: _netArray[netidx]->cellList) {
						if (!_cellArray[cellidx]->lock) {
							if (updatedCell.count(_cellArray[cellidx]) == 0)
								updatedCell[_cellArray[cellidx]] = _cellArray[cellidx]->gain;
							_cellArray[cellidx]->gain--;
						}
					}
				}
				else if (_netArray[netidx]->partCount[fromPart] == 1) {
					for (int cellidx: _netArray[netidx]->cellList) {
						if (!_cellArray[cellidx]->lock && _cellArray[cellidx]->part == fromPart) {
							if (updatedCell.count(_cellArray[cellidx]) == 0)
								updatedCell[_cellArray[cellidx]] = _cellArray[cellidx]->gain;
							_cellArray[cellidx]->gain++;
							break;
						}
					}
				}
			} // End of Update_Gain()

			// update buckets
			for (auto cellIter: updatedCell) {
				Cell4p *cell = cellIter.first;
				Node *node = cell->node;
				int prevGain = cellIter.second;

				// remove from list
				if (_bList[cell->part][prevGain] == node)
					_bList[cell->part][prevGain] = node->next;
				if (node->prev)
					node->prev->next = node->next;
				if (node->next)
					node->next->prev = node->prev;

				// add to list
				node->next = _bList[cell->part][cell->gain];
				node->prev = nullptr;
				if (node->next)
					node->next->prev = node;
				_bList[cell->part][cell->gain] = node;
			}
		} // End of FM pass


		 // roll back to kth path
		for (step--; step > k; step--) {
			Cell4p *cell = _cellArray[_moveStack[step]];
			bool fromPart = cell->part;
			cell->part = !fromPart;
			_partSize[fromPart] -= cell->area[fromPart];
			_partSize[!fromPart] += cell->area[!fromPart];
			for (int netidx : cell->netList) {
				_netArray[netidx]->partCount[fromPart]--;
				_netArray[netidx]->partCount[!fromPart]++;
			}
		}
		_moveStack = vector<int>();

		_cutSize = 0;
		for (Net4p *net : _netArray) {
			if (net->partCount[0] && net->partCount[1])
				_cutSize++;
		}
	} while (_maxAccGain > 0);

	// free memory and write the result back
	rep(i, _cellArray.size()) {
		bool p = _cellArray[i]->part;
		// *********************************************
		// * Storing part info into Instance._tech     *
		// * Libcell is set to die(_tech).tech.libcell *
		// *********************************************
		_data.getINSTANCE(i + 1).setTech(p, _data.getTECHNOLOGY(_data.getDIE(p).getTech()).getLibCell(_data.getINSTANCE(i + 1).getLibCellIndex()));
		delete _cellArray[i]->node;
		delete _cellArray[i];
	}
	for (Net4p *net: _netArray) {
		delete net;
	}
}

bool CellLegalizer::findMaxGain() {
	int slack[2] = { _upperBound[0] - _partSize[0], _upperBound[1] - _partSize[1] };
	int part = slack[0] > slack[1];
	for (int i = _maxPinNum; i >= -_maxPinNum; i--) {		
		rep(j, 2) {
			for (Node *cursor = _bList[part][i]; cursor; cursor = cursor->next) {
				if (_cellArray[cursor->id]->area[!part] > slack[!part])
					continue;
				_maxGainCell = cursor;

				// remove from list
				if (_bList[part][i] == cursor) 
					_bList[part][i] = cursor->next;
				if (cursor->prev)
					cursor->prev->next = cursor->next;
				if (cursor->next)
					cursor->next->prev = cursor->prev;
				return true;
			}
			part = !part;
		}
	}
	return false;
}


double CellLegalizer::rowCost(Row4a *row) {
	row->cost = .0;
	int widthSum(0);
	for (Cluster &cluster: row->clusters)
	for (Cell &cell: cluster.cells) {
		row->cost += sqrt(pow(cell.x - cell.inst->getPosX(), 2) + pow(cell.y - cell.inst->getPosY(), 2));
		widthSum += cell.inst->getLibCell().getSizeX();
	}
	if (widthSum > _rowMaxX[_curDie])
		row->cost = numeric_limits<double>::max();
	return row->cost;
}

void CellLegalizer::placeRow(Row4a *row, Cell *newCell) {
	// First cell or cell does not overlap with last cluster:
	if (row->clusters.empty() || row->clusters.back().x + row->clusters.back().w < newCell->inst->getPosX()) {
		row->clusters.push_back(Cluster(newCell->inst->getPosX()));
		//addCell(row->clusters.back(), newCell);
		Cluster &c = row->clusters.back();
		c.cells.push_back(*newCell);
		c.e += newCell->weight;
		c.q += newCell->weight * (newCell->inst->getPosX() - c.w);
		c.w += newCell->inst->getLibCell().getSizeX();
		collapse(row);
	}
	else {
		//addCell(row->clusters.back(), newCell);
		Cluster &c = row->clusters.back();
		c.cells.push_back(*newCell);
		c.e += newCell->weight;
		c.q += newCell->weight * (newCell->inst->getPosX() - c.w);
		c.w += newCell->inst->getLibCell().getSizeX();
		collapse(row);
	}
	

	// Transform cluster position xc(c) to cell position x(i)
	int x = row->clusters.back().x;
	for (Cell &cell : row->clusters.back().cells) {
		cell.x = x;
		x += cell.inst->getLibCell().getSizeX();
	}

	row->clusters.back().cells.back().y = row->y;
}

void CellLegalizer::collapse(Row4a *row) {
	// Place cluster c:
	Cluster *cluster = &row->clusters.back();
	cluster->x = (double)cluster->q / cluster->e + .5;
	// Limit position between xmin and xmax − wc(c)
	if (cluster->x < _rowMinX[_curDie])
		cluster->x = _rowMinX[_curDie]; else
	if (cluster->x > _rowMaxX[_curDie] - cluster->w)
		cluster->x = _rowMaxX[_curDie] - cluster->w;
	// Overlap between c and its predecessor c'?
	if (row->clusters.size() > 1) {
		Cluster *predC = &row->clusters[row->clusters.size() - 2];
		if (predC->x + predC->w > cluster->x) {
			// Merge cluster c to c':
			//addCluster(row);
			predC->e += cluster->e;
			predC->q += cluster->q - cluster->e * predC->w;
			predC->w += cluster->w;
			predC->cells.insert(predC->cells.end(), cluster->cells.begin(), cluster->cells.end());
			row->clusters.pop_back();
			collapse(row);
		}
	}
}

void CellLegalizer::backup(Row4a *row) {
	row->bak.resize(row->clusters.size());
	rep(i, row->bak.size())
		row->bak[i] = row->clusters[i];
}

void CellLegalizer::restore(Row4a *row) {
	row->clusters.resize(row->bak.size());
	rep(i, row->bak.size())
		row->clusters[i] = row->bak[i];
}

void CellLegalizer::writeback() {
	rep(i, 2)
	for (Row4a *row : _rows[i]) {
		for (Cluster &cluster : row->clusters) {
			for (Cell &cell : cluster.cells) {
				cell.inst->setPos(cell.x, cell.y);

			}
		}
	}
}

void CellLegalizer::sort(int l, int r) {
	if (l < r) {
		int i(l - 1);
		REP(j, l, r) 
		if(_cellList[j]->inst->getPosX() <= _cellList[r]->inst->getPosX())
			swap(_cellList[++i], _cellList[j]);
		swap(_cellList[i + 1], _cellList[r]);
		sort(l, i);
		sort(i + 2, r);
	}
}

void CellLegalizer::plotResult(bool die, Cell *stopCell) {
	ofstream outfile("die" + to_string(die), ios::out);
	plotBoxPLT(outfile, _rowMinX[die], _rowMinY[die], _rowMaxX[die], _rowMaxY[die]);

	for (size_t i = 0; i < _cellList.size(); ++i) {
		if (_cellList[i]->dieIdx != die) continue;
		if (_cellList[i] == stopCell) break;
		plotBoxPLT(outfile, _cellList[i]->inst->getPosX(), _cellList[i]->inst->getPosY(), _cellList[i]->inst->getPosX() + _cellList[i]->inst->getLibCell().getSizeX(), _cellList[i]->inst->getPosY() + _cellList[i]->inst->getLibCell().getSizeY());
	}
	plotBoxPLT(outfile, _rowMinX[die], _rowMinY[die], _rowMinX[die] + _data.getDIE(die).getWidth(), _rowMinY[die] + _data.getDIE(die).getWidth());
	outfile.close();
	//sprintf(cmd, "gnuplot %s", outfilename.c_str());
}

void CellLegalizer::plotBoxPLT(ofstream &stream, int x1, int y1, int x2, int y2)
{
	stream << x1 << " " << y1 << endl
		<< x2 << " " << y2 << endl << endl;
}

bool CellLegalizer::checkResult() {

	// check the die boundary
	for (Cell *cell : _cellList) {
		if (cell->inst->getPosX() < _rowMinX[cell->inst->getTech()] ||
			cell->inst->getPosY() < _rowMinY[cell->inst->getTech()] ||
			cell->inst->getPosX() + cell->inst->getLibCell().getSizeX() > _rowMaxX[cell->inst->getTech()] ||
			cell->inst->getPosY() > _rowMaxY[cell->inst->getTech()])
			return false;
	}

	// check the cells on the same row do not overlap with each other
	int lastX;
	rep(i, 2) for (Row4a *row : _rows[i]) {
		if (!row->clusters.size()) continue;

		lastX = row->clusters[0].cells[0].inst->getPosX() + row->clusters[0].cells[0].inst->getLibCell().getSizeX();
		for (Cluster &cluster : row->clusters) {
			REP(j, 0, cluster.cells.size()) {
				if (&cluster == &(row->clusters[0]) && !j) continue;
				if (cluster.cells[j].inst->getPosX() < lastX) {
					return false;
				}
				lastX = cluster.cells[j].inst->getPosX() + cluster.cells[j].inst->getLibCell().getSizeX();
			}
		}
	}
	return true;
}
