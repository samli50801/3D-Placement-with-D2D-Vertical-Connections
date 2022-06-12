#include "ExampleFunction.h"
#include<cmath>
// minimize 3*x^2 + 2*x*y + 2*y^2 + 7

ExampleFunction::ExampleFunction(Data& data):_data(data)
{
    _diewid = _data.getDIE(0).getWidth();
    double xrange = _diewid;
    double yrange = _data.getDIE(0).getHeight();
    _boundaryArea = xrange * yrange;
    int size = max(yrange, xrange);
    _gamma = size / 21.0;
    size = _data.getINSTANCECount() + 1;
    _expModule.resize(size);
    _binwid = xrange / 12;
    _binhei = yrange / 12;
    double halfw = _binwid / 2;
    double halfh = _binhei / 2;
    for (int i = 0; i < 12; i++)
    {
        _binCenterX[i] = _binwid * i + halfw;
        _binCenterY[i] = _binhei * i + halfh;
    }
    _Tb = (_data.getDIE(0).getMaxUtil() + _data.getDIE(1).getMaxUtil()) / 100.0;
    int numModule = _data.getINSTANCECount();
    for (int i = 0; i < numModule; i++)
    {
        __Instance& m = _data.getINSTANCE(i + 1);
        int mcell = m.getLibCellIndex();
        __Technology tech = _data.getTECHNOLOGY(0);
        __LibCell cell = tech.getLibCell(mcell);
        m.setWidth(cell.getSizeX());
        m.setHight(cell.getSizeY());

        _avgarea += cell.getArea();
    }
    _avgarea/=numModule;
}
void ExampleFunction::evaluateEXP(const vector<double>& x,int numModule)
{
    for (int i = 0; i < numModule; i++)
    {
        double posx = x[2 * i];
        double posy = x[2 * i + 1];
        _expModule[i + 1].px = exp(posx / _gamma);
        _expModule[i + 1].nx = exp(-posx / _gamma);
        _expModule[i + 1].py = exp(posy / _gamma);
        _expModule[i + 1].ny = exp(-posy / _gamma);
    }
}
void ExampleFunction::evaluateFG(const vector<double> &x, double &f, vector<double> &g)
{

    f = 0;
    double totald = 0;
    int numModule = _data.getINSTANCECount();
    for (int i = 0; i < numModule * 2; i++)
    {
        g[i] = 0;
    }
    evaluateEXP(x, numModule);  
    int numNet = _data.getNETCount();
    for (int i = 1; i <= numNet; i++)
    {
        double pExpX = 0, nExpX = 0, pExpY = 0, nExpY = 0;
        __Net& n = _data.getNET(i);
        for (int j = 0; j < n.getNumPin(); j++)
        {
            int id = n.getPin(j).getInst();
            pExpX += _expModule[id].px;
            nExpX += _expModule[id].nx;
            pExpY += _expModule[id].py;
            nExpY += _expModule[id].ny;
        }
        f += log(pExpX) + log(nExpX) + log(pExpY) + log(nExpY);
        for (int j = 0; j < n.getNumPin(); j++)
        {
            int id = n.getPin(j).getInst();
            g[2 * id - 2] += _expModule[id].px / pExpX / _gamma;
            g[2 * id - 2] -= _expModule[id].nx / nExpX / _gamma;
            g[2 * id - 1] += _expModule[id].py / pExpY / _gamma;
            g[2 * id - 1] -= _expModule[id].ny / nExpY / _gamma;
        } 
    }
     //cout<<"1"<<endl;
    for (int binx = 0; binx < 12; binx++)
    {
        for (int biny = 0; biny < 12; biny++)
        {
            double binDesity = 0;
            vector<double>devDensity(numModule * 2, 0);
            for (int i = 0; i < numModule; i++)
            {
                __Instance& m = _data.getINSTANCE(i + 1);
                double c = m.getHight() * m.getWidth() / _avgarea;
                double px, py;

                //px
                double dx = x[2 * i] - _binCenterX[binx];
                dx = abs(dx);
                double wv = m.getWidth();
                if (dx <= wv / 2 + _binwid)
                {
                    double a = 4 / (wv + 2 * _binwid) / (wv + 4 * _binwid);
                    px = 1 - a * dx * dx;
                }
                else if (dx <= wv / 2 + 2 * _binwid)
                {
                    double b = 2 / _binwid / (wv + 4 * _binwid);
                    px = b * (dx - wv / 2 - 2 * _binwid) * (dx - wv / 2 - 2 * _binwid);
                }
                else
                {
                    px = 0;
                }

                //py
                dx = x[2 * i + 1] - _binCenterY[biny];
                dx = abs(dx);
                wv = m.getHight();
                if (dx <= wv / 2 + _binhei)
                {
                    double a = 4 / (wv + 2 * _binhei) / (wv + 4 * _binhei);
                    py = 1 - a * dx * dx;
                }
                else if (dx <= wv / 2 + 2 * _binhei)
                {
                    double b = 2 / _binhei / (wv + 4 * _binhei);
                    py = b * (dx - wv / 2 - 2 * _binhei) * (dx - wv / 2 - 2 * _binhei);
                }
                else
                {
                    py = 0;
                }
                binDesity += c * px * py;
                //diffx
                dx = x[2 * i] - _binCenterX[binx];
                dx = abs(dx);
                wv = m.getWidth();
                if (dx <= wv / 2 + _binwid)
                {
                    double a = 4 / (wv + 2 * _binwid) / (wv + 4 * _binwid);
                    devDensity[2 * i] = - 2 * a * (x[2 * i] - _binCenterX[binx]);
                }
                else if (dx <= wv / 2 + 2 * _binwid)
                {
                    double b = 2 / _binwid / (wv + 4 * _binwid);
                    if (x[2 * i] >= _binCenterX[binx])
                    {
                        devDensity[2 * i] = b * 2 * (x[2 * i] - _binCenterX[binx] - wv / 2 - 2 * _binwid);
                    }
                    else
                    {
                        devDensity[2 * i] = b * 2 * (x[2 * i] - _binCenterX[binx] + wv / 2 + 2 * _binwid);
                    }
                }
                else
                {
                    devDensity[2 * i] = 0;
                }
                devDensity[2 * i] *= py * c;

                //diffy
                dx = x[2 * i + 1] - _binCenterY[biny];
                dx = abs(dx);
                wv = m.getHight();
                if (dx <= wv / 2 + _binhei)
                {
                    double a = 4 / (wv + 2 * _binhei) / (wv + 4 * _binhei);
                    devDensity[2 * i + 1] = - 2 * a * (x[2 * i + 1] - _binCenterY[biny]);
                }
                else if (dx <= wv / 2 + 2 * _binhei)
                {
                    double b = 2 / _binhei / (wv + 4 * _binhei);
                    if (x[2 * i + 1] >= _binCenterY[biny])
                    {
                        devDensity[2 * i + 1] = b * 2 * (x[2 * i + 1] - _binCenterY[biny] - wv / 2 - 2 * _binhei);
                    }
                    else
                    {
                        devDensity[2 * i + 1] = b * 2 * (x[2 * i + 1] - _binCenterY[biny] + wv / 2 + 2 * _binhei);
                    }
                }
                else
                {
                    devDensity[2 * i + 1] = 0;
                }
                devDensity[2 * i + 1] *= px * c;
            }
            for (int i = 0; i < numModule; i++)
            {
                g[2 * i] += _lemda * 2 * (binDesity - _Tb) * devDensity[2 * i];
                g[2 * i + 1] += _lemda * 2 * (binDesity - _Tb) * devDensity[2 * i + 1];
            }
            totald += (binDesity - _Tb) * (binDesity - _Tb);
            //cout << (binDesity - _Tb) * (binDesity - _Tb) << endl;
        }
    }
    _beta = f / totald;
    f += _lemda * totald;
}

void ExampleFunction::evaluateF(const vector<double> &x, double &f)
{
    f = 0;
    double totald = 0;
    int numModule = _data.getINSTANCECount();
    evaluateEXP(x, numModule);  
    int numNet = _data.getNETCount();
    for (int i = 1; i <= numNet; i++)
    {
        double pExpX = 0, nExpX = 0, pExpY = 0, nExpY = 0;
        __Net& n = _data.getNET(i);
        for (int j = 0; j < n.getNumPin(); j++)
        {
            int id = n.getPin(j).getInst();
            pExpX += _expModule[id].px;
            nExpX += _expModule[id].nx;
            pExpY += _expModule[id].py;
            nExpY += _expModule[id].ny;
        }
        f += log(pExpX) + log(nExpX) + log(pExpY) + log(nExpY);
    }

    for (int binx = 0; binx < 12; binx++)
    {
        for (int biny = 0; biny < 12; biny++)
        {
            double binDesity = 0;
            vector<double>devDensity(numModule * 2, 0);
            for (int i = 0; i < numModule; i++)
            {
                __Instance& m = _data.getINSTANCE(i + 1);
                double c = m.getHight() * m.getWidth() / _avgarea;
                double px, py;

                //px
                double dx = x[2 * i] - _binCenterX[binx];
                dx = abs(dx);
                double wv = m.getWidth();
                if (dx <= wv / 2 + _binwid)
                {
                    double a = 4 / (wv + 2 * _binwid) / (wv + 4 * _binwid);
                    px = 1 - a * dx * dx;
                }
                else if (dx <= wv / 2 + 2 * _binwid)
                {
                    double b = 2 / _binwid / (wv + 4 * _binwid);
                    px = b * (dx - wv / 2 - 2 * _binwid) * (dx - wv / 2 - 2 * _binwid);
                }
                else
                {
                    px = 0;
                }

                //py
                dx = x[2 * i + 1] - _binCenterY[biny];
                dx = abs(dx);
                wv = m.getHight();
                if (dx <= wv / 2 + _binhei)
                {
                    double a = 4 / (wv + 2 * _binhei) / (wv + 4 * _binhei);
                    py = 1 - a * dx * dx;
                }
                else if (dx <= wv / 2 + 2 * _binhei)
                {
                    double b = 2 / _binhei / (wv + 4 * _binhei);
                    py = b * (dx - wv / 2 - 2 * _binhei) * (dx - wv / 2 - 2 * _binhei);
                }
                else
                {
                    py = 0;
                }
                binDesity += c * px * py;
     
            }
            totald += (binDesity - _Tb) * (binDesity - _Tb);
        }
    }
    _beta = f / totald;
    f += _lemda * totald;
}

unsigned ExampleFunction::dimension()
{
    return 2 * _data.getINSTANCECount(); // num_blocks*2 
    // each two dimension represent the X and Y dimensions of each block
}
