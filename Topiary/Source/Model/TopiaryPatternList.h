/////////////////////////////////////////////////////////////////////////////
/*
This file is part of Topiary, Copyright Tom Tollenaere 2018-20.

Topiary is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Topiary is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Topiary. If not, see <https://www.gnu.org/licenses/>.
*/
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "TopiaryListModel.h"

#ifdef TOPIARYMODEL
class TOPIARYMODEL; // needs to be a defined model!

class TopiaryPatternList : public TopiaryListModel
{ 

public:
	TopiaryPatternList();
	~TopiaryPatternList();

	void setModel(TOPIARYMODEL* m);
	void sortByID() override; // sort by index n (order in which the variables are in the struct
	void del(int n) override;
	void add() override;
	void duplicate(int i);
	void addToModel(XmlElement *model);
	void getFromModel(XmlElement *model);
	void validateTableEdit(int p, XmlElement* child, String attribute) override;
	static const int maxItems = MAXNOPATTERNS;
	
	struct data // MUST match what has been defined in the headerlist data!!!
	{
		int ID;
		String name;
		int measures;
	}; 
	
	data dataList[maxItems];
	 
	void fillDataList(XmlElement* dList) override;
	
	void setIntByIndex(int row, int o, int newInt) override;
	
	void setStringByIndex(int row, int i, String newString) override;
	

private:
	TOPIARYMODEL* m;

	void swap(int from, int to)
	{
		intSwap(dataList[from].ID, dataList[to].ID);
		intSwap(dataList[from].measures, dataList[to].measures);
		stringSwap(dataList[from].name, dataList[to].name);
	} // swap
	
}; // TopiaryPatternList

#endif