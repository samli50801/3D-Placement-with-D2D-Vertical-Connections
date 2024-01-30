#include<iostream>
#include<string>
#include<fstream>
#include"data.h"
#include"GlobalPlacer.h"
#include "CellLegalizer.h"
#include "viaHandler.h"

void plotBoxPLT(ofstream& stream, double x1, double y1, double x2, double y2);
void plotPlacementResult(Data& data, vector<Via>vias,string name);
int main(int argc, char* argv[])
{
	Data data(argv[1]);
	GlobalPlacer gp(data);
	gp.place();
	//gp.plotPlacementResult(argv[2]);
	/*Cell Legalize*/
	CellLegalizer cellLegalizer(data);
	cellLegalizer.legalize();

	//DP
	//cout<<"in DP\n";
	data.DP(1);
	//cout<<"out DP\n";
	/*Via Legalize*/
	ViaHandler viaHandler(data);
	//viaHandler.placeVia();
	//viaHandler.plot();
	viaHandler.legalize1();
	//data._vias=viaHandler.getViaVec();
	cout<<"1"<<endl;
	plotPlacementResult(data,viaHandler.getViaVec(),argv[2]);
	cout<<"2"<<endl;
	//viaHandler.plot();
	data.output(argv[2], viaHandler.getViaInfo());
	cout<<"3"<<endl;
}
void plotPlacementResult(Data& data, vector<Via>vias,string name)
{

    ofstream outfile(name+"INSTANCE1.plt", ios::out);
    outfile << " " << endl;
    outfile << "set size ratio 1" << endl;
    outfile << "set nokey" << endl << endl;
    outfile << "plot[:][:] '-' w l lt 3 lw 2, '-' w l lt 1" << endl << endl;
    outfile << "# bounding box" << endl;
    plotBoxPLT(outfile, 0, 0, data.getDIE(0).getWidth(), data.getDIE(0).getHeight());
    outfile << "EOF" << endl;
    outfile << "# modules" << endl << "0.00, 0.00" << endl << endl;

    ofstream outfile2(name+"INSTANCE2.plt", ios::out);
    outfile2 << " " << endl;
    outfile2 << "set size ratio 1" << endl;
    outfile2 << "set nokey" << endl << endl;
    outfile2 << "plot[:][:] '-' w l lt 3 lw 2, '-' w l lt 1" << endl << endl;
    outfile2 << "# bounding box" << endl;
    plotBoxPLT(outfile2, 0, 0, data.getDIE(0).getWidth(), data.getDIE(0).getHeight());
    outfile2 << "EOF" << endl;
    outfile2 << "# modules" << endl << "0.00, 0.00" << endl << endl;

    for (size_t i = 1; i <= data.getINSTANCECount(); ++i)
    {
        __Instance inst = data.getINSTANCE(i);
        __LibCell cell = inst.getLibCell();
	if(inst.getTech()==0)
        	 plotBoxPLT(outfile, inst.getPosX(), inst.getPosY(), inst.getPosX() + cell.getSizeX(), inst.getPosY() + cell.getSizeY());
	else
		plotBoxPLT(outfile2, inst.getPosX(), inst.getPosY(), inst.getPosX() + cell.getSizeX(), inst.getPosY() + cell.getSizeY());
    }
    outfile << "EOF" << endl;
    outfile << "pause -1 'Press any key to close.'" << endl;
    outfile.close();

outfile2 << "EOF" << endl;
    outfile2 << "pause -1 'Press any key to close.'" << endl;
    outfile2.close();

    ofstream outfile1(name+"VIA.plt", ios::out);
    outfile1 << " " << endl;
    outfile1 << "set size ratio 1" << endl;
    outfile1 << "set nokey" << endl << endl;
    outfile1 << "plot[:][:] '-' w l lt 3 lw 2, '-' w l lt 1" << endl << endl;
    outfile1 << "# bounding box" << endl;
    plotBoxPLT(outfile1, 0, 0, data.getDIE(0).getWidth(), data.getDIE(0).getHeight());
    outfile1 << "EOF" << endl;
    outfile1 << "# modules" << endl << "0.00, 0.00" << endl << endl;


    for (size_t i = 0; i < vias.size(); ++i)
    {
        Via& via = vias[i];
        plotBoxPLT(outfile1, via.getx1(), via.gety1(), via.getx2(), via.gety2());
    }
    outfile1 << "EOF" << endl;
    outfile1 << "pause -1 'Press any key to close.'" << endl;
    outfile1.close();


}


void plotBoxPLT(ofstream& stream, double x1, double y1, double x2, double y2)
{
    stream << x1 << ", " << y1 << endl << x2 << ", " << y1 << endl
        << x2 << ", " << y2 << endl << x1 << ", " << y2 << endl
        << x1 << ", " << y1 << endl << endl;
}
