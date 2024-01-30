#define _GLIBCXX_USE_CXX11_ABI 0
#ifndef EXAMPLEFUNCTION_H
#define EXAMPLEFUNCTION_H

#include "NumericalOptimizerInterface.h"
#include "data.h"
struct expmodule
{
    double px, nx, py, ny;
};
class ExampleFunction : public NumericalOptimizerInterface
{
public:
    ExampleFunction(Data& data);

    void evaluateFG(const vector<double> &x, double &f, vector<double> &g);
    void evaluateF(const vector<double> &x, double &f);
    void evaluateEXP(const vector<double>& x, int numModule);
    double getBeta() { return _beta; }
    void setLemda(double l) { _lemda = l; }
    unsigned dimension();
private:
    Data& _data;
    double _gamma;
    vector<expmodule>_expModule;
    int _diewid;
    int _tech0;
    int _tech1;
    double _binwid, _binhei;
    double _binCenterX[12];
    double _binCenterY[12];
    double _boundaryArea;
    double _Tb;
    double _lemda;
    double _beta;
double _avgarea;
};
#endif // EXAMPLEFUNCTION_H
