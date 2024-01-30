#include "CG.h"
#include "../lp_solve/lp_lib.h"
#include <numeric>

using namespace cg;

bool CG::startBufferAreaOpt = false;
bool CG::startBufferAreaLeg = false;
vector<Component*> CG::lastDeletedPoint;

bool AreaComp(const Point* a, const Point* b) {
    return a->_comp->getArea() < b->_comp->getArea();
}

Point::Point(Component* comp)
{
    /* pseudo source and sink */
    if (comp == NULL) {
        Component *pseudoComp = new Component();
        pseudoComp->type = FIXED;
        _comp = pseudoComp;
    }else 
    /* macros */
    {  
        _comp = comp;
        _lm = -1;
        _rm = INT_MAX;
        _dm = -1;
        _um = INT_MAX;
    }  
}

void CG::initialize()
{
    /* _pointVec initialization */
    _pointVec.clear();
    _pointVec.reserve(_compVec.size() + 4);

    /* component to point */
    for (size_t i = 0; i < _compVec.size(); ++i) {
        Point *newPoint = new Point(_compVec[i]);
        newPoint->_no = i;
        _pointVec.push_back(newPoint);
    }

    /* pseudopoint initialization */
    _hsource = new Point(NULL);
    _hsource->_no = _pointVec.size();
    _hsource->_comp->name = "_hsource";
    _hsource->_comp->setArg(_designLeft, _designDown, 0, _designUp - _designDown);

    _hsink = new Point(NULL);
    _hsink->_no = _pointVec.size() + 1;
    _hsink->_comp->name = "_hsink";
    _hsink->_comp->setArg(_designRight, _designDown, 0, _designUp - _designDown);

    _vsource = new Point(NULL);
    _vsource->_no = _pointVec.size() + 2;
    _vsource->_comp->name = "_vsource";
    _vsource->_comp->setArg(_designLeft, _designDown, _designRight - _designLeft, 0);

    _vsink = new Point(NULL);
    _vsink->_no = _pointVec.size() + 3;
    _vsink->_comp->name = "_vsink";
    _vsink->_comp->setArg(_designLeft, _designUp, _designRight - _designLeft, 0);
}

int CG::getRelation(Point* i, Point* j) // j is on the i's xxx
{
    int right = j->_comp->get_ll_x() - i->_comp->get_ur_x();  //then j is Totally to right Of i +
    int left = i->_comp->get_ll_x() - j->_comp->get_ur_x();   //then j is Totally to left Of i +
    int up = j->_comp->get_ll_y() - i->_comp->get_ur_y();     //then j is Totally above i +
    int down = i->_comp->get_ll_y() - j->_comp->get_ur_y();   //then j is Totally below i +

    /* upper-right */
    if (right >= 0 && up >= 0) {
        if (right >= up)
            return RIGHT;
        else
            return UP;
    }
    /* lower-right */
    else if (right >= 0 && down >= 0) {
if (right >= down)
            return RIGHT;
else
   return DOWN;
}
    /* upper-left */
    else if (left >= 0 && up >= 0) {
        if (left >= up)
            return LEFT;
        else
            return UP;
    }
    /* lower-left */
    else if (left >= 0 && down >= 0) {
        if (left >= down)
            return LEFT;
        else
            return DOWN;
    }
    /* right */
    else if (right >= 0) return RIGHT;
    /* up */
else if (up >= 0) return UP;
    /* down */
else if (down >= 0) return DOWN;
    /* left */
else if (left >= 0) return LEFT;
    /* overlap */
    else if (!(right>=0 || left>=0 || up>=0 || down>=0)) {

        Component* a = i->_comp;
        Component* b = j->_comp;
        double xOverlap = (a->getWidth() + b->getWidth()) / 2.0 - fabs(a->get_mid_x() - b->get_mid_x());
        double yOverlap = (a->getHeight() + b->getHeight()) / 2.0 - fabs(a->get_mid_y() - b->get_mid_y());
        if (xOverlap <= yOverlap) {
            double right = fabs(a->get_ll_x() - b->get_ur_x());
   double left = fabs(a->get_ur_x() - b->get_ll_x());
            // i is right of j
   if (right < left)
   return LEFT;
   else
                return RIGHT;
        }else {
            double up = fabs(a->get_ll_y() - b->get_ur_y());
   double down = fabs(a->get_ur_y() - b->get_ll_y());
   // i is above j
   if (up < down)
                return DOWN;
   else
                return UP;
        }
    }
}


void CG::buildGraph()
{
    /* build the TCG */
    /* total n*(n-1)/2 comparisons */
    for (size_t i = 0, end_i = _pointVec.size() - 1; i < end_i; ++i) {
        for (size_t j = i + 1, end_j = _pointVec.size(); j < end_j; ++j) {
            /* j is on i which direction */
            int relation = getRelation(_pointVec[i], _pointVec[j]);

            /* edge weight */
            int piWidth = _pointVec[i]->_comp->width + _minSpace;
            int piHeight = _pointVec[i]->_comp->height + _minSpace;
            int pjWidth = _pointVec[j]->_comp->width + _minSpace;
            int pjHeight = _pointVec[j]->_comp->height + _minSpace;

            /* build edge */
            switch (relation)
            {
            case LEFT:
                _pointVec[j]->_right.push_back(make_pair(i, pjWidth));
                _pointVec[i]->_left.push_back(make_pair(j, pjWidth));
                break;
            case RIGHT:
                _pointVec[i]->_right.push_back(make_pair(j, piWidth));
                _pointVec[j]->_left.push_back(make_pair(i, piWidth));
                break;
            case DOWN:
                _pointVec[j]->_up.push_back(make_pair(i, pjHeight));
                _pointVec[i]->_down.push_back(make_pair(j, pjHeight));
                break;
            case UP:
                _pointVec[i]->_up.push_back(make_pair(j, piHeight));
                _pointVec[j]->_down.push_back(make_pair(i, piHeight));
                break;
                
            default:
                break;
            }
        }
    }

    /* connect the node who has zero InDegree(OutDegree) to the source(sink) */
    for (size_t i = 0, end_i = _pointVec.size(); i < end_i; ++i) {
        if (_pointVec[i]->_left.empty()) {
            _hsource->_right.push_back(make_pair(i, _hsource->_comp->width));
            _pointVec[i]->_left.push_back(make_pair(_hsource->_no, _hsource->_comp->width));
        }
        if (_pointVec[i]->_right.empty()) {
            _hsink->_left.push_back(make_pair(i, _pointVec[i]->_comp->getWidth()));
            _pointVec[i]->_right.push_back(make_pair(_hsink->_no, _pointVec[i]->_comp->getWidth()));
        }
        if (_pointVec[i]->_down.empty()) {
            _vsource->_up.push_back(make_pair(i, _vsource->_comp->height));
            _pointVec[i]->_down.push_back(make_pair(_vsource->_no, _vsource->_comp->height));
        }
        if (_pointVec[i]->_up.empty()) {
            _vsink->_down.push_back(make_pair(i, _pointVec[i]->_comp->getHeight()));
            _pointVec[i]->_up.push_back(make_pair(_vsink->_no, _pointVec[i]->_comp->getHeight()));
        }
    }

    /* put pseudo point in the last 4 position */
    _pointVec.push_back(_hsource);
    _pointVec.push_back(_hsink);
    _pointVec.push_back(_vsource);
    _pointVec.push_back(_vsink);
}

int CG::getHLongestPath(Point* start, Point *(&end))
{
    /* using topology sort */
    end = _hsink;

    queue<size_t> Q;
    int inDegree[_pointVec.size()];
    memset(inDegree, 0, _pointVec.size()*sizeof(int));

    int latestStart[_pointVec.size()];
    std::fill_n(latestStart, _pointVec.size(), start->_comp->get_ll_x());

    /* set start point */
    Q.push(start->_no);
    start->_criticalHParent = NULL;

    size_t index;
    Point *p;

    while (!Q.empty()) {

        index = Q.front();
        p = _pointVec[index];
        Q.pop();

        if (p->_comp->type == FIXED && p != _hsource && p != _hsink) {
            //check if macros touch the fixed block's left side 
            if (latestStart[index] > p->_comp->_llx) {
                end = p;
                return p->_comp->_llx - latestStart[index];
            }
            else
                latestStart[index] = p->_comp->_llx;
        }

        for (vector<pair<size_t, int>>::iterator it = p->_right.begin(); it != p->_right.end(); it++) {
            size_t rIndex = it->first;
            if (latestStart[index] + it->second >= latestStart[rIndex]) {
                
                latestStart[rIndex] = latestStart[index] + it->second;
                _pointVec[rIndex]->_criticalHParent = p;
            }

            if (++inDegree[rIndex] == _pointVec[rIndex]->_left.size() || p == start) 
                Q.push(rIndex);
        }
    }

    return _designRight - latestStart[_hsink->_no];
}

int CG::getVLongestPath(Point* start, Point *(&end))
{
    /* using topology sort */
    end = _vsink;

    queue<size_t> Q;
    int inDegree[_pointVec.size()];
    memset(inDegree, 0, _pointVec.size()*sizeof(int));

    int latestStart[_pointVec.size()];
    std::fill_n(latestStart, _pointVec.size(), start->_comp->get_ll_y());

    /* set start point */
    Q.push(start->_no);
    start->_criticalVParent = NULL;

    size_t index;
    Point *p;

    while (!Q.empty()) {

        index = Q.front();
        p = _pointVec[index];
        Q.pop();

        if (p->_comp->type == FIXED && p != _vsource && p != _vsink) {
            // check if macros touch the fixed block's buttom 
            if (latestStart[index] > p->_comp->_lly) {
                end = p;
                return p->_comp->_lly - latestStart[index];
            }
            else
                latestStart[index] = p->_comp->_lly;
        }

        for (vector<pair<size_t, int>>::iterator it = p->_up.begin(); it != p->_up.end(); it++) {
            size_t uIndex = it->first;
            if (latestStart[index] + it->second >= latestStart[uIndex]) {
                latestStart[uIndex] = latestStart[index] + it->second;
                _pointVec[uIndex]->_criticalVParent = p;
            }

            if (++inDegree[uIndex] == _pointVec[uIndex]->_down.size() || p == start)
                Q.push(uIndex);
        }
    }
    return _designUp - latestStart[_vsink->_no];
}

void CG::deleteEdge(vector<pair<size_t, int>>& adjEdge, size_t no)
{
    vector<pair<size_t, int>>::iterator deleteEdge;
    for (vector<pair<size_t, int>>::iterator it = adjEdge.begin(); it != adjEdge.end();) {
        if (it->first == no)
            it = adjEdge.erase(it);
        else 
            ++it;
    }
}

void CG::deleteEdge(vector<pair<size_t, int>>& adjEdge, Point* p)
{
    vector<pair<size_t, int>>::iterator deleteEdge;
    for (vector<pair<size_t, int>>::iterator it = adjEdge.begin(); it != adjEdge.end();) {
        if (_pointVec[it->first] == p) {
            it = adjEdge.erase(it);
            break;
        }
        else 
            ++it;
    }
}

void CG::deletePoint(Point* p, bool graphType)
{
    //cout << "delete point: " << p->_comp->name << endl;
	if (!startBufferAreaLeg)
		deletedPoint.push_back(p->_comp);

    /* cannot delete fixed block */
    if (p->_comp->type == FIXED) 
        return;

    /*if (graphType == H) {*/
        for (vector<pair<size_t, int>>::iterator i = p->_right.begin(); i != p->_right.end(); ++i) {

            Point *rightNeighbor = _pointVec[i->first];

            deleteEdge(rightNeighbor->_left, p);
        
            if (rightNeighbor->_left.empty() && rightNeighbor != _hsink) {
                _hsource->_right.push_back(make_pair(i->first, _hsource->_comp->getWidth()));
                rightNeighbor->_left.push_back(make_pair(_hsource->_no, _hsource->_comp->getWidth()));
            }
        }
        p->_right.clear();

        for (vector<pair<size_t, int>>::iterator i = p->_left.begin(); i != p->_left.end(); ++i) {

            Point *leftNeighbor = _pointVec[i->first];

            deleteEdge(leftNeighbor->_right, p);

            if (leftNeighbor->_right.empty() && leftNeighbor != _hsource) {
                _hsink->_left.push_back(make_pair(i->first, leftNeighbor->_comp->getWidth()));
                leftNeighbor->_right.push_back(make_pair(_hsink->_no, leftNeighbor->_comp->getWidth()));
            }
        }
        p->_left.clear();
    /*}
    else {*/
        for (vector<pair<size_t, int>>::iterator i = p->_up.begin(); i != p->_up.end(); ++i) {

            Point *upNeighbor = _pointVec[i->first];

            deleteEdge(upNeighbor->_down, p);

            if (upNeighbor->_down.empty() && upNeighbor != _vsink) {
                _vsource->_up.push_back(make_pair(i->first, _vsource->_comp->getHeight()));
                upNeighbor->_down.push_back(make_pair(_vsource->_no, _vsource->_comp->getHeight()));
            }
        }
        p->_up.clear();

        for (vector<pair<size_t, int>>::iterator i = p->_down.begin(); i != p->_down.end(); ++i) {

            Point* downNeighbor = _pointVec[i->first];

            deleteEdge(downNeighbor->_up, p);

            if (downNeighbor->_up.empty() && downNeighbor != _vsource) {
                _vsink->_down.push_back(make_pair(i->first, downNeighbor->_comp->getHeight()));
                downNeighbor->_up.push_back(make_pair(_vsink->_no, downNeighbor->_comp->getHeight()));
            }
        }
        p->_down.clear();
    /*}*/
}


void CG::solveLPbyCG()
{
    lprec *lp;
    int Ncol, *colno = NULL, j, ret = 0;
    REAL *row = NULL;

    /* We will build the model row by row
     So we start with creating a model with 0 rows and 2 columns */
    /* macro's x and y / macro's x and y 's absolute / macro's x and y 's initial position */
    Ncol = 2 * _pointVec.size() + 2 * _pointVec.size();    // # of variables
cout<<"a0"<<endl;
    lp = make_lp(0, Ncol);
    if(lp == NULL)
        ret = 1; /* couldn't construct a new model... */
cout<<"a1"<<endl;
    /* create space large enough for one row */
    colno = (int *) malloc(Ncol * sizeof(*colno));
    row = (REAL *) malloc(Ncol * sizeof(*row));

    set_add_rowmode(lp, TRUE);  /* makes building the model faster if it is done rows by row */
cout<<"a2"<<endl;
    /* construct first row (120 x + 210 y <= 15000) */
    /* right constraint*/
    for (size_t i = 0; i < _pointVec.size(); ++i) {
	cout<<i<<endl;
	int a=0;
        if (i == _vsource->_no || i == _vsink->_no) 
            continue;
        if (_pointVec[i]->_comp->type == FIXED) {
            j = 0;
            colno[j] = 2 * i + 1;
            row[j++] = 1;
            add_constraintex(lp, j, row, colno, EQ, _pointVec[i]->_comp->get_ll_x());
        }
        for (vector<pair<size_t, int>>::iterator r = _pointVec[i]->_right.begin(); r != _pointVec[i]->_right.end(); ++r) {
            // right - i >= edge weight
	cout<<i<<": "<<a++<<endl;
            j = 0;
            colno[j] = 2 * (*r).first + 1;
            row[j++] = 1;
            colno[j] = 2 * i + 1;
            row[j++] = -1;
            // add the row to lpsolve 
            add_constraintex(lp, j, row, colno, GE, (*r).second);
        }
    }
cout<<"a"<<endl;
    /* up constraint*/  
    for (size_t i = 0; i < _pointVec.size(); ++i) {
        if (i == _hsource->_no || i == _hsink->_no) 
            continue;
        if (_pointVec[i]->_comp->type == FIXED) {
            j = 0;
            colno[j] = 2 * i + 2;
            row[j++] = 1;
            add_constraintex(lp, j, row, colno, EQ, _pointVec[i]->_comp->get_ll_y());
        }
        for (vector<pair<size_t, int>>::iterator u = _pointVec[i]->_up.begin(); u != _pointVec[i]->_up.end(); ++u) {
            // up - i >= edge weight
            j = 0;
            colno[j] = 2 * (*u).first + 2;
            row[j++] = 1;
            colno[j] = 2 * i + 2;
            row[j++] = -1;
            // add the row to lpsolve 
            add_constraintex(lp, j, row, colno, GE, (*u).second);
        }
    }
cout<<"b"<<endl;
    /* absolute setting*/
    for (size_t i = 0; i < _pointVec.size(); ++i) {

        size_t absIndex = 2 * _pointVec.size() + 2 * i;
        size_t newPosIndex = 2 * i;

        j = 0;
        colno[j] = absIndex + 1;
        row[j++] = -1;
        colno[j] = newPosIndex + 1;
        row[j++] = 1;
        add_constraintex(lp, j, row, colno, LE, _pointVec[i]->_comp->_llx);
        j = 0;
        colno[j] = absIndex + 1;
        row[j++] = 1;
        colno[j] = newPosIndex + 1;
        row[j++] = 1;
        add_constraintex(lp, j, row, colno, GE, _pointVec[i]->_comp->_llx);

        j = 0;
        colno[j] = absIndex + 2;
        row[j++] = -1;
        colno[j] = newPosIndex + 2;
        row[j++] = 1;
        add_constraintex(lp, j, row, colno, LE, _pointVec[i]->_comp->_lly);
        j = 0;
        colno[j] = absIndex + 2;
        row[j++] = 1;
        colno[j] = newPosIndex + 2;
        row[j++] = 1;
        add_constraintex(lp, j, row, colno, GE, _pointVec[i]->_comp->_lly);
    }
  
    set_add_rowmode(lp, FALSE); /* rowmode should be turned off again when done building the model */
cout<<"c"<<endl;
    /* minimize displacement */
    j = 0;
    for (size_t i = 0, end_i = _pointVec.size() - 4; i < end_i; ++i) {
        size_t absIndex = 2 * _pointVec.size() + 2 * i;
        colno[j] = absIndex + 1;
        row[j++] = 1;
        colno[j] = absIndex + 2;
        row[j++] = 1;
    }
cout<<"d"<<endl;
    /* set the objective in lpsolve */
    if(!set_obj_fnex(lp, j, row, colno))
      ret = 4;
  
    /* set the object direction to maximize */
    set_minim(lp);

    /* Now let lpsolve calculate a solution */
    ret = solve(lp);

    /* a solution is calculated, now lets get some results */

    /* variable values */
    get_variables(lp, row);
cout<<"e"<<endl;
    for (size_t i = 0, end_i = _pointVec.size() - 4; i < end_i; ++i) {
        if (_compVec[i]->type == FIXED)
            continue;
        _compVec[i]->_llx = row[2 * i];
        _compVec[i]->_lly = row[2 * i + 1];
    }
    /* we are done now */
cout<<"f"<<endl;
    /* free allocated memory */
    if(row != NULL)
        free(row);
    if(colno != NULL)
        free(colno);

    if(lp != NULL) {
        /* clean up such that all used memory by lpsolve is freed */
        delete_lp(lp);
    }
}


std::vector<Point*>::iterator getBestCandidate(vector<Point*>& criticalList)
{
    size_t smallest = 0;
    double cost = DBL_MAX;

    for (size_t nth = 0; nth < criticalList.size(); ++nth) {
        if (criticalList[nth]->_comp->getType() != FIXED
        && criticalList[nth]->_comp->getArea() < cost) {
            smallest = nth;
            cost = criticalList[nth]->_comp->getArea();
        }
    }

    return criticalList.begin() + smallest;
}


void CG::legalizeCriticalPath()
{
    int designWidth = _designRight - _designLeft;
    int designHeight = _designUp - _designDown;
    int longestHPath;
    int longestVPath;
    Point *h_end, *v_end;
    
    /* update longest path length */
    longestHPath = getHLongestPath(_hsource, h_end);
    longestVPath = getVLongestPath(_vsource, v_end);
    
    while (longestHPath < 0) {
        // extract the macros in critical path
        vector<Point*> criticalList;
        Point *current = h_end;
        while ((current = current->_criticalHParent)->_comp->getType() != FIXED) {
            criticalList.push_back(current);
        }

        // select the macro with smallest area
        std::vector<Point*>::iterator victimPoint = getBestCandidate(criticalList);
        // if macro is bigger than threashold => move operation 
        if ((*victimPoint)->_comp->getArea() > (double)designWidth * (double)designHeight * 1.01
            && criticalList.size() >= 2) {

            // select reduction edge to move 
            Point *left, *right;
            if (victimPoint == criticalList.begin()) {
                left = *(victimPoint + 1);
                right = *victimPoint;
            }
            else if (victimPoint == criticalList.end() - 1) {
                left = *victimPoint;
                right = *(victimPoint - 1);
            }
            else if ((*(victimPoint + 1))->_comp->getHeight() < (*(victimPoint - 1))->_comp->getHeight()) {
                left = *(victimPoint + 1);
                right = *victimPoint;
            }
            else {
                left = *victimPoint;
                right = *(victimPoint - 1);
            }

            if (left->_comp->getType() != FIXED && right->_comp->getType() != FIXED) {

            }
            else {
                if (left->_comp->getType() == FIXED) 
                    deletePoint(right, H);
                else
                    deletePoint(left, H);
            }
        }
        // else => delete operation
        else 
            deletePoint(*victimPoint, H);

        longestHPath = getHLongestPath(_hsource, h_end);
    }

    while (longestVPath < 0) {

        // extract the macros in critical path
        vector<Point*> criticalList;
        Point *current = v_end;    // the start point of critical path
        while ((current = current->_criticalVParent)->_comp->getType() != FIXED) {
            criticalList.push_back(current);
        }

        std::vector<Point*>::iterator victimPoint = getBestCandidate(criticalList);

        if ((*victimPoint)->_comp->getArea() > (double)designWidth * (double)designHeight * 1.01 //0.01
            && criticalList.size() >= 2) {
            
            // select reduction edge to move
            Point *down, *up;
            if (victimPoint == criticalList.begin()) {
                down = *(victimPoint + 1);
                up = *victimPoint;
            }
            else if (victimPoint == criticalList.end() - 1) {
                down = *victimPoint;
                up = *(victimPoint - 1);
            }
            else if ((*(victimPoint + 1))->_comp->getWidth() < (*(victimPoint - 1))->_comp->getWidth()) {
                down = *(victimPoint + 1);
                up = *victimPoint;
            }
            else {
                down = *victimPoint;
                up = *(victimPoint - 1);
            }

            if (down->_comp->getType() != FIXED && up->_comp->getType() != FIXED) {

            }
            else {
                if (down->_comp->getType() == FIXED) 
                    deletePoint(up, V);
                else
                    deletePoint(down, V);
            }
        }
        else 
            deletePoint(*victimPoint, V);

        longestVPath = getVLongestPath(_vsource, v_end);
    }
}

void CG::legalize()
{
	do {
		/* static*/
		lastDeletedPoint = deletedPoint;
cout<<"1"<<endl;
		/* reset source*/
		deletedPoint.clear();
		/* initialize */
		initialize();
cout<<"2"<<endl;
		/* build TCG */
		buildGraph();
cout<<"3"<<endl;
		/* delete point to make TCG fit in the boundary */
		legalizeCriticalPath();
cout<<"4"<<endl;
		/* linear programming solver */
		solveLPbyCG();
cout<<"5"<<endl;

		/* reset source*/
		delete _hsource;
		_hsource = NULL;
		delete _vsource;
		_vsource = NULL;
		delete _hsink;
		_hsink = NULL;
		delete _vsink;
		_vsink = NULL;

	} while (lastDeletedPoint != deletedPoint);
}
