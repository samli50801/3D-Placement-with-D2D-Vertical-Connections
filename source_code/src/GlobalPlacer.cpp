#include "GlobalPlacer.h"
#include "ExampleFunction.h"
#include "NumericalOptimizer.h"
#include<cmath>

GlobalPlacer::GlobalPlacer(Data& datab):_data(datab)
{

}


void GlobalPlacer::place()
{
    ExampleFunction ef(_data); 
    vector<double> x(2 * _data.getINSTANCECount());
    int width = _data.getDIE(0).getWidth();
    int height = _data.getDIE(0).getHeight();
    for (int i = 0; i < _data.getINSTANCECount(); i++)
    {
        x[2 * i] = rand() % width;
        x[2 * i + 1] = rand() % height;
    }
    double size = (width + height) * 8;
    NumericalOptimizer no(ef);
    no.setX(x);
    no.setNumIteration(15); // user-specified parameter
    no.setStepSizeBound(size); // user-specified parameter
    //cout<<"num"<<endl;
    no.solve(); // Conjugate Gradient solver
    //cout<<"num"<<endl;
    no.setNumIteration(25); // user-specified parameter
    no.setStepSizeBound(size / 2); // user-specified parameter
    no.solve(); // Conjugate Gradient solver
    double b = ef.getBeta();
    cout<<"success "<<b<<endl;
    size/=8;
    for (int i = 1; i < 4; i++)
    {
string name="round";
        for (int j = 0; j < _data.getINSTANCECount(); j++)
        {
            x[2 * j] = no.x(2 * j);
            x[2 * j + 1] = no.x(2 * j + 1);
            __Instance& m = _data.getINSTANCE(j + 1);
            while (x[2 * j] + m.getWidth() > width)
            {
                x[2 * j] = width-m.getWidth() - rand() % ((width-m.getWidth()) / 3);
            }
            while (x[2 * j + 1] + m.getHight() > height)
            {
                x[2 * j + 1] = height-  m.getHight() - rand() % ((height-m.getHight()) / 3);
            }
	    while (x[2 * j] < 0)
            {
                x[2 * j] = rand() % (width / 3);
            }
            while (x[2 * j + 1] < 0)
            {
                x[2 * j + 1] = rand() % (height / 3);
            }
        }
name+=to_string(i)+".plt";
 	plotPlacementResult(name,x);
        no.setX(x);
        ef.setLemda(b * pow(2, i));
        no.setNumIteration(40); // user-specified parameter
        no.setStepSizeBound(size); // user-specified parameter
        no.solve(); // Conjugate Gradient solver
    }
    for (int j = 0; j < _data.getINSTANCECount(); j++)
    {
        x[2 * j] = no.x(2 * j);
        x[2 * j + 1] = no.x(2 * j + 1);
        __Instance& m = _data.getINSTANCE(j + 1);
        while (x[2 * j] + m.getWidth() > width)
        {
            x[2 * j] = width-m.getWidth() - rand() % ((width-m.getWidth()) / 4);
        }
        while (x[2 * j + 1] + m.getHight() > height)
        {
            x[2 * j + 1] = height-  m.getHight() - rand() % ((height-m.getHight()) / 4);
        }
        while (x[2 * j] < 0)
        {
            x[2 * j] = rand() % (width / 4);
        }
        while (x[2 * j + 1] < 0)
        {
            x[2 * j + 1] = rand() % (height / 4);
        }
        _data.getINSTANCE(j + 1).setPos(x[2 * j], x[2 * j - 1]);
    }
plotPlacementResult("round4.plt",x);
}
void GlobalPlacer::plotPlacementResult(string name, vector<double> x)
{
    ofstream outfile(name, ios::out);
    outfile << " " << endl;
    outfile << "set size ratio 1" << endl;
    outfile << "set nokey" << endl << endl;
    outfile << "plot[:][:] '-' w l lt 3 lw 2, '-' w l lt 1" << endl << endl;
    outfile << "# bounding box" << endl;
    plotBoxPLT(outfile, 0, 0, _data.getDIE(0).getWidth(), _data.getDIE(0).getHeight());
    outfile << "EOF" << endl;
    outfile << "# modules" << endl << "0.00, 0.00" << endl << endl;


    for (int j = 0; j < _data.getINSTANCECount(); j++)
    {
        __Instance inst = _data.getINSTANCE(j+1);
         plotBoxPLT(outfile,x[2*j], x[2*j+1], x[2*j]+inst.getWidth(), x[2*j+1]+inst.getHight());
    }
    outfile << "EOF" << endl;
    outfile << "pause -1 'Press any key to close.'" << endl;
    outfile.close();

    /*ofstream outfile1("VIA.plt", ios::out);
    outfile1 << " " << endl;
    outfile1 << "set size ratio 1" << endl;
    outfile1 << "set nokey" << endl << endl;
    outfile1 << "plot[:][:] '-' w l lt 3 lw 2, '-' w l lt 1" << endl << endl;
    outfile1 << "# bounding box" << endl;
    plotBoxPLT(outfile1, 0, 0, _data.getDIE(0).getWidth(), _data.getDIE(0).getHeight());
    outfile1 << "EOF" << endl;
    outfile1 << "# modules" << endl << "0.00, 0.00" << endl << endl;


    for (size_t i = 0; i < _data._vias.size(); ++i)
    {
        Via& via = _data._vias[i];
        plotBoxPLT(outfile1, via.getx1(), via.gety1(), via.getx2(), via.gety2());
    }
    outfile1 << "EOF" << endl;
    outfile1 << "pause -1 'Press any key to close.'" << endl;
    outfile1.close();
*/

}


void GlobalPlacer::plotBoxPLT(ofstream& stream, double x1, double y1, double x2, double y2)
{
    stream << x1 << ", " << y1 << endl << x2 << ", " << y1 << endl
        << x2 << ", " << y2 << endl << x1 << ", " << y2 << endl
        << x1 << ", " << y1 << endl << endl;
}
