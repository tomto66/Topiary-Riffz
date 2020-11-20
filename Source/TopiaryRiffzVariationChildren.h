/////////////////////////////////////////////////////////////////////////////
/*
This file is part of Topiary Riffz, Copyright Tom Tollenaere 2018-20. 

Topiary Riffz is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Topiary Riffz is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Topiary Riffz. If not, see <https://www.gnu.org/licenses/>.
*/
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include"TopiaryRiffzModel.h"
#include "../../Topiary/Source/Components/TopiaryVariationChildren.h"
#include "../../Topiary/Source/Components/TopiaryTable.h"

class NoteAssignmentComponent : public Component
{
public:

	TopiaryLookAndFeel rndLookAndFeel;
	TopiaryTable noteAssignmentTable;
	XmlElement* noteAssignmentListHeader = nullptr;
	XmlElement* noteAssignmentListData = nullptr;
	
	int width = 450;
	int heigth = 300;

	NoteAssignmentComponent();
	~NoteAssignmentComponent();
	void resized();
	void paint(Graphics& g) override;
	void setParent(TopiaryRiffzVariationComponent* p);
	ComboBox patternCombo;
	TextButton newButton;
	TextButton deleteButton;
	TextButton saveButton;

	TopiaryMidiLearnEditor noteEditor;
	TextEditor offsetEditor;

private:

	TopiaryRiffzVariationComponent* parent;
	static const int tH = 200;
	static const int tW = 200;

	static const int buttonH = 20;
	static const int buttonW = 150;
	static const int editorH = 20;
	static const int editorW = 35;
	
	//static const int sliderH = 100;
	//static const int sliderV = 65;
	//static const int sliderTW = 60;
	//static const int sliderTH = 15;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteAssignmentComponent)

}; // NoteAssignmentComponent
