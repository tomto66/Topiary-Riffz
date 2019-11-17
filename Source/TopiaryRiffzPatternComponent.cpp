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

#include "../JuceLibraryCode/JuceHeader.h"
#include "TopiaryRiffzPatternComponent.h"


TopiaryRiffzPatternComponent::TopiaryRiffzPatternComponent()
{   // size set by TabbedComponent!
	// Table for patterns
	patternTable.setSize(patternTW, patternTH);
	addAndMakeVisible(patternTable);

	addAndMakeVisible(patternCombo);
	patternCombo.setSize(200, 30);
	patternCombo.onChange = [this]
	{
		processPatternCombo();
	};

	addAndMakeVisible(patternLengthComponent);
	addAndMakeVisible(actionButtonsComponent);

} // TopiaryRiffzPatternComponent

/////////////////////////////////////////////////////////////////////////


TopiaryRiffzPatternComponent::~TopiaryRiffzPatternComponent()
{
	riffzModel->removeListener((ActionListener*)this);
} //~TopiaryRiffzPatternComponent

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzPatternComponent::setModel(TopiaryRiffzModel* m)
{
	riffzModel = m;
	riffzModel->setListener((ActionListener*)this);

	patternTable.setModel(riffzModel->getPattern(0)); // (0) just to get it started because it will have to validate note data!
	actionButtonsComponent.setParent(this);
	patternLengthComponent.setParent(this);

	riffzModel->setListener((ActionListener*)this);

	// trick to call the model and read 

	actionListenerCallback(MsgPatternList); // among other things, set the pattern combobox;

} // setModel

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzPatternComponent::paint(Graphics& g)
{
	auto area = getLocalBounds();

	g.fillAll(TopiaryColour::background);
	g.setColour(TopiaryColour::foreground);
	g.setFont(12.0f);
	patternTable.setBounds(160, 10, patternTW, patternTH);

	patternCombo.setBounds(600, 25, 240, 30);

	patternLengthComponent.setBounds(600, 60, patternLengthComponent.getWidth(), patternLengthComponent.getHeight());
	actionButtonsComponent.setBounds(600, 110, actionButtonsComponent.getWidth(), actionButtonsComponent.getHeight());

} // paint

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzPatternComponent::resized()
{
} //resized

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzPatternComponent::actionListenerCallback(const String &message)
{
	if (message.compare(MsgLoad) == 0)
	{
		// this means we just loaded a preset; model needs to be refreshed because the pattern data may have been overwritten
		//riffzModel->getPatternModel(0, &patternListHeader, &patternListData);  // @ initialization this will simply be an empty pattern
		//patternTable.setDataLists(patternListHeader, patternListData);
		patternTable.setModel(riffzModel->getPattern(0)); // (0) just to get it started because it will have to validate note data!
	}
	else if (message.compare(MsgPattern) == 0)
	{
		// pattern (may have) changed; update the table
		int rememberSelectedRow = patternTable.getSelectedRow();
		patternTable.updateContent();
		patternTable.selectRow(rememberSelectedRow);
		setButtonStates(); // is in include file
	}
	else if (message.compare(MsgPatternList) == 0)
	{
		// find the list of patterns loaded
		String patterns[8];
		int rememberPatternComboSelection = patternCombo.getSelectedItemIndex() - 1;
		if (rememberPatternComboSelection < 0)
			rememberPatternComboSelection = 0;

		patternCombo.clear();
		for (int i = 0; i < 8; i++) patterns[i] = "";

		riffzModel->getPatterns(patterns);

		for (int i = 0; i < 8; i++)
		{
			if (patterns[i].compare(""))
				patternCombo.addItem(patterns[i], i + 1);
		}

		// if there are no patterns, disable yourself!!!
		if (riffzModel->getNumPatterns() > 0)
		{
			this->setEnabled(true);
			if (patternCombo.getNumItems() > rememberPatternComboSelection)
			{
				patternCombo.setSelectedItemIndex(rememberPatternComboSelection);
			}
			else
			{
				patternCombo.setSelectedItemIndex(0);
			}

			//actionListenerCallback(MsgPattern);  // force reload of patterndata
		}
		else
		{
			this->setEnabled(false);
			patternCombo.setSelectedItemIndex(0, dontSendNotification);
		}
		// fill the combobox with the pattern names
	}

} // actionListenerCallback

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzPatternComponent::setPatternLength()
{
	// validate the input
	int len = patternLengthComponent.measureEditor.getText().getIntValue();
	if ((len <= 0) || len > 16)
	{
		riffzModel->Log("Invalid pattern length.", Topiary::LogType::Warning);
		return;
	}
	else
	{
		riffzModel->setPatternLength(patternCombo.getSelectedId() - 1, len, patternLengthComponent.keepTail.getToggleState());
		patternTable.updateContent();
	}

} // setPatternLength

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzPatternComponent::deleteNote()
{
	// deletes selected note

	auto selection = patternTable.getSelectedRow();
	jassert(selection >= 0);

	riffzModel->deleteNote(patternCombo.getSelectedId() - 1, selection);
	//patternTable.updateContent(); not needed, model will post action MsgPattern

} // deleteNote

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzPatternComponent::addNote()
{
	// adds a note at the position selected in table
	int timestamp = 0;
	int note = 0;
	int velocity;
	int length;

	// see if there is something selected in the table
	if (patternTable.getNumRows() == 0)
		timestamp = 0;
	else if (patternTable.getSelectedRow() < 0)
		timestamp = 0;
	else
	{
		riffzModel->getNote(patternCombo.getSelectedId() - 1, patternTable.getSelectedRow(), note, velocity, timestamp, length);
	}
	riffzModel->addNote(patternCombo.getSelectedId() - 1, 0, 127, Topiary::TicksPerQuarter, timestamp);
	

	actionListenerCallback(MsgPattern); // sets all buttons properly (e.g. enable the delete button) - will remember the selected row!

}  // addNote

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzPatternComponent::copyNote()
{
	// adds a note at the position selected in table
	int timestamp = 0;

	riffzModel->getNote(patternCombo.getSelectedId() - 1, patternTable.getSelectedRow(), pbufferNote, pbufferVelocity, timestamp, pbufferLength);

	// call setButtonStates so pase button comes on
	setButtonStates();

}  // copyNote

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzPatternComponent::pasteNote()
{
	// adds a note at the position selected in table
	int timestamp = 0;
	int length, note, velocity;

	jassert(pbufferNote != -1);

	// see if there is something selected in the table
	if (patternTable.getNumRows() == 0)
		timestamp = 0;
	else if (patternTable.getSelectedRow() < 0)
		timestamp = 0;
	else
	{
		riffzModel->getNote(patternCombo.getSelectedId() - 1, patternTable.getSelectedRow(), note, velocity, timestamp, length);
	}
	riffzModel->addNote(patternCombo.getSelectedId() - 1, pbufferNote, pbufferVelocity, pbufferLength, timestamp);


}  // pasteNote

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzPatternComponent::clearPattern()
{
	riffzModel->clearPattern(patternCombo.getSelectedId() - 1);
	actionListenerCallback(MsgPattern);

} // clearPattern

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzPatternComponent::deleteAllNotes()
{
	// deletes all notes equal to selected one from the pattern
	riffzModel->deleteAllNotes(patternCombo.getSelectedId() - 1, patternTable.getSelectedRow() + 1);
	actionListenerCallback(MsgPattern);

} // deleteAllNotes

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzPatternComponent::quantize()
{
	int ticks = 0;
	switch (actionButtonsComponent.quantizeCombo.getSelectedId())
	{
	case 1:
		ticks = Topiary::TicksPerQuarter;
		break;
	case 2:
		ticks = Topiary::TicksPerQuarter/2;
		break;
	case 3:
		ticks = Topiary::TicksPerQuarter/3;
		break;
	case 4:
		ticks = Topiary::TicksPerQuarter/4;
		break;
	default: ticks = Topiary::TicksPerQuarter / 5;
		break;

	}

	riffzModel->quantize(patternCombo.getSelectedId() - 1, ticks);
	actionListenerCallback(MsgPattern);
} // quantize