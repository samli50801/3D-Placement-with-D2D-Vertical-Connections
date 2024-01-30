#include"data.h"

Data::Data(string fileName)
{
	fstream file;
	file.open(fileName, ios::in);
	string temp;
	char tempc;
	//NumTechnologies
	file >> temp >> _technologyCount;
	_technologies.resize(_technologyCount);
	for (int i = 0; i < _technologyCount; i++)
	{
		string name;
		int libCellCount;
		file >> temp >> name >> libCellCount;
		_technologies[i].setTechName(name);
		_technologies[i].setLibCellCount(libCellCount);
		int x, y;
		for (int j = 1; j <= libCellCount; j++)
		{
			__LibCell cell;
			int pinCount;
			file >> temp >> temp >> x >> y >> pinCount;
			cell.setSize(x, y);
			cell.setPinCount(pinCount);
			for (int k = 1; k <= pinCount; k++)
			{
				__PinLocation location;
				file >> temp >> temp >> location._x >> location._y;
				cell.addPin(k, location);
			}
			_technologies[i].addLibCell(j, cell);
		}

	}

	//DieSize
	int lx, ly, ux, uy;
	file >> temp >> lx >> ly >> ux >> uy;
	__Die die(lx, ly, ux, uy);

	//TopDieMaxUtil
	int u;
	file >> temp >> u;
	die.setMaxUtil(u);
	_dies.push_back(die);

	//BottomDieMaxUtil
	file >> temp >> u;
	die.setMaxUtil(u);
	_dies.push_back(die);

	//TopDieRows
	int startX, startY, rowLength, rowHeight, repeatCount;
	file >> temp >> startX >> startY >> rowLength >> rowHeight >> repeatCount;
	__Row tr(startX, startY, rowLength, rowHeight, repeatCount);
	_dies[0].setRow(tr);

	//BottomDieRows
	file >> temp >> startX >> startY >> rowLength >> rowHeight >> repeatCount;
	__Row br(startX, startY, rowLength, rowHeight, repeatCount);
	_dies[1].setRow(br);

	//TopDieTech
	string TechName;
	file >> temp >> TechName;
	if (_technologies[0].getName() == TechName)
	{
		_dies[0].setTech(0);
	}
	else
	{
		_dies[0].setTech(1);
	}

	//BottomDieTech
	file >> temp >> TechName;
	if (_technologies[0].getName() == TechName)
	{
		_dies[1].setTech(0);
	}
	else
	{
		_dies[1].setTech(1);
	}

	//TerminalSize
	file >> temp >> _terminalSizeX >> _terminalSizeY;

	//TerminalSpacing
	file >> temp >> _terminalSpacing;

	//NumInstances
	file >> temp >> _instanceCount;
	_instances.resize(_instanceCount + 1);
	int index;
	for (int i = 1; i <= _instanceCount; i++)
	{
		file >> temp >> temp >> tempc >> tempc >> index;
		_instances[i].setLibCellIndex(index);
	}
	//NumNets
	file >> temp >> _netCount;
	_nets.resize(_netCount + 1);
	for (int i = 1; i <= _netCount; i++)
	{
		int num;
		file >> temp >> temp >> num;
		_nets[i].setNumPins(num);
		for (int j = 0; j < num; j++)
		{
			int cell, pin;
			file >> temp >> tempc >> cell >> tempc >> tempc >> pin;
			__Pin p(cell, pin);
			_nets[i].addPin(j, p);
			
			_instances[cell].addNet(i); // new
		}
	}
}
void Data::output(string fileName, vector<pair<int, pair<int, int>>> viaInfo)
{
	fstream file;
	file.open(fileName, ios::out);

	vector <int> top;
	vector <int> bottom;
	for (int i = 1; i <= _instanceCount; i++)
	{
		if (_instances[i].getTech() == 0)
			top.push_back(i);
		else
			bottom.push_back(i);
	}

	file << "TopDiePlacement " << top.size() << '\n';
	for (int i = 0; i < top.size(); i++)
		file << "Inst C" << top[i] << ' ' << _instances[top[i]].getPosX() << ' ' << _instances[top[i]].getPosY() << '\n';

	//file << '\n';
	file << "BottomDiePlacement " << bottom.size() << '\n';
	for (int i = 0; i < bottom.size(); i++)
		file << "Inst C" << bottom[i] << ' ' << _instances[bottom[i]].getPosX() << ' ' 
				<< _instances[bottom[i]].getPosY() << '\n';

	// terminals
	int viaNum = viaInfo.size();
	file << "NumTerminals " << viaNum << endl;
	for (size_t i = 0; i < viaNum; ++i) {
		file << "Terminal N" << viaInfo[i].first << " " << viaInfo[i].second.first << " " << viaInfo[i].second.second << endl;
	}


}
