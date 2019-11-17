/////////////////////////////////////////////////////////////////////////////
/*
This file is part of Topiary Riffz, Copyright Tom Tollenaere 2018-19.

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
#include"../../Topiary/Source/TopiaryTable.h"  
#include "TopiaryRiffzPatternChildren.h"

class TopiaryRiffzPatternComponent : public Component, ActionListener
{
public:
	TopiaryRiffzPatternComponent();
	~TopiaryRiffzPatternComponent();

	void paint(Graphics&) override;
	void resized() override;
	void setModel(TopiaryRiffzModel* m);
	void actionListenerCallback(const String &message) override;
	void setPatternLength(); 
	void deleteNote(); // deletes selected note
	void addNote(); // adds a note at the position selected in table
	void copyNote();
	void pasteNote();
	void clearPattern();
	void deleteAllNotes(); // deletes all notes equal to selected one from pattern
	void quantize();

private:
	TopiaryRiffzModel* riffzModel;
	TopiaryTable patternTable;
	int patternTW = 380;
	int patternTH = 348;
	
	ComboBox patternCombo;
	PatternLengthComponent patternLengthComponent;
	ActionButtonsComponent actionButtonsComponent;

	// copy/paste buffer
	int pbufferNote = -1;			// -1 means uninitialized
	int pbufferVelocity = -1;
	int pbufferLength = -1;

	///////////////////////////////////////////////////////////////////////////////////////

	void processPatternCombo() // call when pattern combobox changed
	{
		if (patternCombo.getSelectedId()>0)
			patternTable.setModel(riffzModel->getPattern(patternCombo.getSelectedId() - 1));
		else
			patternTable.setModel(riffzModel->getPattern(0));

		patternTable.updateContent();
		patternTable.setPattern(patternCombo.getSelectedId() - 1);
		
		patternLengthComponent.measureEditor.setText(String(riffzModel->getPatternLengthInMeasures( patternCombo.getSelectedId() - 1)));
		
		setButtonStates();

		riffzModel->setPatternSelectedInPatternEditor(patternCombo.getSelectedId() - 1); // needed so that transport can check whether recording makes sense

	} // processPatternCombo

	///////////////////////////////////////////////////////////////////////////////////////

	void setButtonStates()
	{
		int numRows = patternTable.getNumRows();

		if (pbufferNote == -1)
			actionButtonsComponent.pasteButton.setEnabled(false);
		else
			actionButtonsComponent.pasteButton.setEnabled(true);

		// make sure that if there are notes, at least one is selected
		if (patternTable.getSelectedRow() <0)
			if (numRows > 0)
				patternTable.selectRow(0);

		// called from actionListener; enable/dcisable buttons depending on pattern state
		if (patternTable.getNumRows() == 0)
		{		
			actionButtonsComponent.deleteButton.setEnabled(false);
			actionButtonsComponent.copyButton.setEnabled(false);
			actionButtonsComponent.deleteAllNotesButton.setEnabled(false);
		}
		else if (patternTable.getSelectedRow() >= 0)
		{
			actionButtonsComponent.deleteButton.setEnabled(true);
			actionButtonsComponent.copyButton.setEnabled(true);
			actionButtonsComponent.deleteAllNotesButton.setEnabled(true);
		}
		else
		{
			// nothing selected
			actionButtonsComponent.deleteButton.setEnabled(false);
			actionButtonsComponent.copyButton.setEnabled(false);
			actionButtonsComponent.deleteAllNotesButton.setEnabled(false);
		}

	}  // setButtonStates

	///////////////////////////////////////////////////////////////////////////////////////

	

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopiaryRiffzPatternComponent)
};

/////////////////////////////////////////////////////////////////////
