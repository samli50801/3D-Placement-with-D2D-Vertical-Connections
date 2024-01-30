#include <random>
#include "viaHandler.h"
#include "CG.h"

#define EMP 0
#define OCP 1

void ViaHandler::placeVia()
{
    srand(0);

    for (size_t i = 1; i <= _data.getNETCount(); ++i) {
        __Net& net = _data.getNET(i);

        // bounding box of pins in index 0(TopDIE)/1(BottomDIE)
        int minX[2] = {INT_MAX, INT_MAX};
        int minY[2] = {INT_MAX, INT_MAX};
        int maxX[2] = {INT_MIN, INT_MIN};
        int maxY[2] = {INT_MIN, INT_MIN};
        
        for (size_t j = 0; j < net.getNumPin(); ++j) {

            __Pin& pin = net.getPin(j);
            __Instance& instance = _data.getINSTANCE(pin.getInst());

            int pinX = instance.getPosX() + instance.getLibCell().getPin(pin.getLibPin())._x;
            int pinY = instance.getPosY() + instance.getLibCell().getPin(pin.getLibPin())._y;

            bool part = instance.getTech(); // 0(TopDIE) 1(BottomDIE)

            minX[part] = std::min(minX[part], pinX);
            minY[part] = std::min(minY[part], pinY);
            maxX[part] = std::max(maxX[part], pinX);
            maxY[part] = std::max(maxY[part], pinY);
        }

        // all pins are in the same die
        if (minX[0] == INT_MAX || minX[1] == INT_MAX) 
            continue;

        // bounding box that via should insert in
        int viaBoxX1 = min(max(minX[0], minX[1]), min(maxX[0], maxX[1]));   // lower-left x
        int viaBoxX2 = max(max(minX[0], minX[1]), min(maxX[0], maxX[1]));   // upper-right x
        int viaBoxY1 = min(max(minY[0], minY[1]), min(maxY[0], maxY[1]));   // lower-left y
        int viaBoxY2 = max(max(minY[0], minY[1]), min(maxY[0], maxY[1]));   // upper-right y
        
        // random assign the via position in viaBox
        int viaCentX = rand() % (viaBoxX2 - viaBoxX1 + 1) + viaBoxX1;
        int viaCentY = rand() % (viaBoxY2 - viaBoxY1 + 1) + viaBoxY1;
        _viaVec.push_back(Via(viaCentX, viaCentY, _data.getTerminalSizeX(), _data.getTerminalSizeY(), i));
    }
}

void ViaHandler::legalize1()
{
    int dieWidth = _data.getDIE(0).getWidth() - _data.getTerminalSpacing();
    int dieHeight = _data.getDIE(0).getHeight() - _data.getTerminalSpacing();
    int gridW = _data.getTerminalSizeX() + _data.getTerminalSpacing();
    int gridH = _data.getTerminalSizeY() + _data.getTerminalSpacing();
    int gridHNum = dieWidth / gridW;
    int gridVNum = dieHeight / gridH;
    _grid = new bool*[gridVNum];
    for (int i = 0; i < gridVNum; ++i) {
        _grid[i] = new bool[gridHNum]();
    }

    srand(0);
    int count = 0, count1 = 0;

    for (size_t i = 1; i <= _data.getNETCount(); ++i) {
        __Net& net = _data.getNET(i);

        // bounding box of pins in index 0(TopDIE)/1(BottomDIE)
        int minX[2] = {INT_MAX, INT_MAX};
        int minY[2] = {INT_MAX, INT_MAX};
        int maxX[2] = {INT_MIN, INT_MIN};
        int maxY[2] = {INT_MIN, INT_MIN};
        
        for (size_t j = 0; j < net.getNumPin(); ++j) {

            __Pin& pin = net.getPin(j);
            __Instance& instance = _data.getINSTANCE(pin.getInst());

            int pinX = instance.getPosX() + instance.getLibCell().getPin(pin.getLibPin())._x;
            int pinY = instance.getPosY() + instance.getLibCell().getPin(pin.getLibPin())._y;

            bool part = instance.getTech(); // 0(TopDIE) 1(BottomDIE)

            minX[part] = std::min(minX[part], pinX);
            minY[part] = std::min(minY[part], pinY);
            maxX[part] = std::max(maxX[part], pinX);
            maxY[part] = std::max(maxY[part], pinY);
        }

        // all pins are in the same die
        if (minX[0] == INT_MAX || minX[1] == INT_MAX) 
            continue;

        // bounding box that via should insert in
        int viaBoxX1 = min(max(minX[0], minX[1]), min(maxX[0], maxX[1]));   // lower-left x
        int viaBoxX2 = max(max(minX[0], minX[1]), min(maxX[0], maxX[1]));   // upper-right x
        int viaBoxY1 = min(max(minY[0], minY[1]), min(maxY[0], maxY[1]));   // lower-left y
        int viaBoxY2 = max(max(minY[0], minY[1]), min(maxY[0], maxY[1]));   // upper-right y
        
        // random assign the via position in viaBox
        int viaCentX = rand() % (viaBoxX2 - viaBoxX1 + 1) + viaBoxX1;
        int viaCentY = rand() % (viaBoxY2 - viaBoxY1 + 1) + viaBoxY1;

        int gridHIndex = viaCentX / gridW;
        int gridVIndex = viaCentY / gridH;
        gridHIndex = gridHIndex >= gridHNum ? gridHNum-1 : gridHIndex;
        gridVIndex = gridVIndex >= gridVNum ? gridVNum-1 : gridVIndex;

        if (_grid[gridVIndex][gridHIndex] == EMP) {
            int posX = gridW*gridHIndex + _data.getTerminalSpacing() + _data.getTerminalSizeX()/2;
            int posY = gridH*gridVIndex + _data.getTerminalSpacing() + _data.getTerminalSizeY()/2;
            _viaVec.push_back(Via(posX, posY, _data.getTerminalSizeX(), _data.getTerminalSizeY(), i));
            _grid[gridVIndex][gridHIndex] = OCP;
        } 
        else {
            int d = 0;
            int l;
            bool found = 0;
            while (!found) {
                ++d;
                l = 2*d + 1;
                int aStart = gridVIndex-(l-1)/2 < 0 ? 0 : gridVIndex-(l-1)/2;
                int aEnd = gridVIndex+(l-1)/2 >= gridVNum ? gridVNum-1 : gridVIndex+(l-1)/2;
                int bStart = gridHIndex-(l-1)/2 < 0 ? 0 : gridHIndex-(l-1)/2;
                int bEnd = gridHIndex+(l-1)/2 >= gridHNum ? gridHNum-1 : gridHIndex+(l-1)/2;
                //cout << aStart << " " << aEnd << " " << bStart << " " << bEnd << endl;
                /*if (aStart == 0 && aEnd == gridVNum-1)
                    break;*/
                for (int a = aStart; a <= aEnd; ++a) {
                    if (found)
                        break;
                    for (int b = bStart; b <= bEnd; ++b) {
                        if (a >= gridVNum || b >= gridHNum) 
                            continue;
                        //cout << a << "  " << b << endl;
                        if (_grid[a][b] == EMP) {
                            int posX = gridW*b + _data.getTerminalSpacing() + _data.getTerminalSizeX()/2;
                            int posY = gridH*a + _data.getTerminalSpacing() + _data.getTerminalSizeY()/2;
                            _viaVec.push_back(Via(posX, posY, _data.getTerminalSizeX(), _data.getTerminalSizeY(), i));
                            _grid[a][b] = OCP;
                            found = 1;
                            break;
                        }
                    }
                }
            }
            
        }
    }
}

/* A via is <relation> to B via */
int ViaHandler::getRelation(Via& a, Via& b)
{
    int xOverlap = min(a.getx2(), b.getx2()) - max(a.getx1(), b.getx1());
    int yOverlap = min(a.gety2(), b.gety2()) - max(a.gety1(), b.gety1());
    if (xOverlap > 0 && yOverlap > 0) {
        return xOverlap < yOverlap ? a.getx2() < b.getx2() ? LEFT : RIGHT : a.gety2() < b.gety2() ? DOWN : UP;
    } 
    else if (xOverlap > 0) {
        return a.gety1() < b.gety1() ? DOWN : UP;
    } 
    else if (yOverlap > 0) {
        return a.getx1() < b.getx1() ? LEFT : RIGHT;
    } 
    else {
        return abs(xOverlap) > abs(yOverlap) ? a.getcx() < b.getcx() ? LEFT : RIGHT : a.getcy() < b.getcy() ? DOWN : UP;
    }
}

void ViaHandler::setData(Parser& p, vector<Component*> &comp)
{
    p.dbuPerMicron = 1;
    /* shrink boundary for not letting via compact to the original boundary */
    int x1 = _data.getDIE(0).getLowerLeftX() + _data.getTerminalSpacing();
    int x2 = _data.getDIE(0).getUpperRightX() - _data.getTerminalSpacing();
    int y1 = _data.getDIE(0).getLowerLeftY() + _data.getTerminalSpacing();
    int y2 = _data.getDIE(0).getUpperRightY() - _data.getTerminalSpacing();
    p.dieArea.push_back(make_pair(x1, y1));
    p.dieArea.push_back(make_pair(x2, y2));
    p.numComps = _viaVec.size();
    int i = 0;
    for (Via& via: _viaVec) {
        Component *c = new Component;
        c->name = "name";
        c->width = _data.getTerminalSizeX();
        c->height = _data.getTerminalSizeY();
        c->type = PLACED;
        c->_llx = via.getx1();
        c->_lly = via.gety1();
        c->_cx = via.getcx();
        c->_cy = via.getcy();
       comp.push_back(c);
    }
    p._pwc = 0;
    p._baredc = 0;
    p._mcsbmc = _data.getTerminalSpacing();

    // construct Bound object for left, right, top, down boundary
    int leftX = p.dieArea[0].first, rightX = p.dieArea[1].first, downY = p.dieArea[0].second, topY = p.dieArea[1].second;
    Bound *left = new Bound(leftX, downY, leftX, topY);
    p.bound.push_back(left);
    Bound *top = new Bound(leftX, topY, rightX, topY);
    p.bound.push_back(top);
    Bound *right = new Bound(rightX, topY, rightX, downY);
    p.bound.push_back(right);
    Bound *down = new Bound(rightX, downY, leftX, downY);
    p.bound.push_back(down);
    p.minOfWidth = leftX;
    p.minOfHeight = downY;
    p.maxOfWidth = rightX;
    p.maxOfHeight = topY;
}

void ViaHandler::legalize()
{
    int viaNum = _viaVec.size();
    Parser parser;
    vector<Component*> comp;
    /* transform the current data to CG's data */
    setData(parser, comp);
    /* legalize overlapping */
    cg::CG cg(comp, parser);
    cg.legalize();
    /* CG's data are assined to the current data */
    for (size_t i = 0; i < viaNum; ++i) {
        _viaVec[i].setcx(comp[i]->get_mid_x());
        _viaVec[i].setcy(comp[i]->get_mid_y());
    }
}

vector<pair<int, pair<int, int>>> ViaHandler::getViaInfo()
{
    int visNum = _viaVec.size();
    vector<pair<int, pair<int, int>>> info(visNum);
    for (size_t i = 0; i < visNum; ++i) {
        info[i].first = _viaVec[i].getNetId();
        info[i].second.first = _viaVec[i].getcx();
        info[i].second.second = _viaVec[i].getcy();
    }
    return info;
}

void ViaHandler::plot() {
	__Die& die = _data.getDIE(0);
	/////////////info. to show for gnu/////////////
	int boundWidth = die.getWidth();
    int boundHeight = die.getHeight();
 	/////////////////////////////////////////////
 	//gnuplot preset
	fstream outgraph("gnuplot/via.gp", ios::out);
	outgraph << "reset\n";
	outgraph << "set tics\n";
	outgraph << "unset key\n";
	outgraph << "set title \"The result of Via Legalization" << "\"\n";
	outgraph << "set size " << 1 << ", " << 1 << endl;
	int index = 1;
 	// wirte block info into output.gp

	outgraph << "set object " << index << " rect from "
			<< die.getLowerLeftX() << "," << die.getLowerLeftY() << " to "
			<< die.getUpperRightX() << "," << die.getUpperRightY() << " fc rgb 'black'\n";
	index++;

    for (Via& via : _viaVec) {
        int x0 = via.getx1();
		int y0 = via.gety1();
		int x1 = via.getx2();
		int y1 = via.gety2();
        outgraph << "set object " << index << " rect from " 
		  		<< x0 << "," << y0 << " to " << x1 << "," << y1 << " fs empty border fc rgb 'red'\n";
		index++;
    }

	fstream outline("gnuplot/line", ios::out);

	outgraph << "set style line 1 lc rgb \"red\" lw 3\n";
	outgraph << "set border ls 1\n";
	outgraph << "set terminal png\n";
	outgraph << "set output \"gnuplot/via.png\"\n";
	outgraph << "plot [" << die.getLowerLeftX() << ":" << die.getUpperRightX() << "]["
		<< die.getLowerLeftY() << ":" << die.getUpperRightY() << "]\'gnuplot/line\' w l lt 2 lw 1\n";
	outgraph << "set terminal x11 persist\n";
	outgraph << "replot\n";
	outgraph << "exit";
	outgraph.close();

	int x = system("gnuplot gnuplot/via.gp");
}
