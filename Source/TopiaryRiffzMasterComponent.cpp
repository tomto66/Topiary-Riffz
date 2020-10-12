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
#include "TopiaryRiffzModel.h"
#include "TopiaryRiffzMasterComponent.h"


TopiaryRiffzMasterComponent::TopiaryRiffzMasterComponent()
{   // size set by TabbedComponent!

	////////////////////////////////////////
	// Pattern stuff
	////////////////////////////////////////

	// Table for patterns
	patternsTable.setSize(patternTW, patternTH);
	addAndMakeVisible(patternsTable);

	//  Insert Pattern button
	insertPatternButton.setSize(buttonW, buttonH);
	addAndMakeVisible(insertPatternButton);
	insertPatternButton.setButtonText("Load MIDI file");
	insertPatternButton.onClick = [this] {
		auto selection = patternsTable.getSelectedRow();
		jassert(selection >= 0);

		riffzModel->insertPatternFromFile(selection, false); // false means make new pattern
		
		riffzModel->sendActionMessage(MsgPatternList); // tables resort the data!
	};

	// Duplicate Pattern button
	duplicatePatternButton.setSize(buttonW, buttonH);
	addAndMakeVisible(duplicatePatternButton);
	duplicatePatternButton.setButtonText("Duplicate pattern");
	duplicatePatternButton.onClick = [this] {

		auto selection = patternsTable.getSelectedRow();
		jassert(selection >= 0);

		riffzModel->duplicatePattern(selection);
		patternsTable.updateContent();
		
		riffzModel->sendActionMessage(MsgPatternList); // tables resort the data!
	};

	// Delete Pattern button
	deletePatternButton.setSize(buttonW, buttonH);
	addAndMakeVisible(deletePatternButton);
	deletePatternButton.setButtonText("Delete pattern");
	deletePatternButton.onClick = [this] {

		auto selection = patternsTable.getSelectedRow();
		jassert(selection >= 0);
		
		riffzModel->deletePattern(selection);
		patternsTable.updateContent();
		riffzModel->sendActionMessage(MsgPatternList); // tables resort the data!
	};


	// Add Pattern button
	newPatternButton.setSize(buttonW, buttonH);
	addAndMakeVisible(newPatternButton);
	newPatternButton.setButtonText("Add pattern");
	newPatternButton.onClick = [this] {
		riffzModel->addPattern();
		patternsTable.updateContent();
		riffzModel->sendActionMessage(MsgPatternList); // tables resort the data!
		patternsTable.selectRow(riffzModel->getNumPatterns()-1);   // select the new row
	};

	// Overload Pattern button
	overloadPatternButton.setSize(buttonW, buttonH);
	addAndMakeVisible(overloadPatternButton);
	overloadPatternButton.setButtonText("Overload");
	overloadPatternButton.onClick = [this] {
		auto selection = patternsTable.getSelectedRow();
		jassert(selection >= 0);

		riffzModel->insertPatternFromFile(selection, true); // true means overload

		riffzModel->sendActionMessage(MsgPatternList); // tables resort the data!
	};

	
	setButtonStates();

	/////////////////////////////////
	// Settings
	/////////////////////////////////

	addAndMakeVisible(settingComponent);
	settingComponent.setParent(this);

	
} // TopiaryRiffzMasterComponent() 

/////////////////////////////////////////////////////////////////////////

TopiaryRiffzMasterComponent::~TopiaryRiffzMasterComponent()
{
	riffzModel->removeListener((ActionListener*)this);
} // ~TopiaryRiffzMasterComponent

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzMasterComponent::setModel(TopiaryRiffzModel* m)
{
	riffzModel = m;
	
	patternsTable.setModel(riffzModel->getPatternList());
	
	riffzModel->setListener((ActionListener*)this);
	settingComponent.keyRangeFromEditor.setModel(riffzModel, Topiary::LearnMidiId::keyrangeTo);
	settingComponent.keyRangeFromEditor.setModel(riffzModel, Topiary::LearnMidiId::keyrangeFrom);
	actionListenerCallback(MsgPatternList);
	actionListenerCallback(MsgTransport);

}

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzMasterComponent::paint(Graphics& g)
{
	int lineSize = 2;
	int labelOffset = 15;
	int patternButtonOffsetX = 270 + 93;
	int patternBlockOffsetX = 93;
	g.fillAll(TopiaryColour::background);
	g.setColour(TopiaryColour::foreground);
	g.setFont(12.0f);

	//// Patterns

	g.drawText("Patterns", patternBlockOffsetX, 10, 200, labelOffset, juce::Justification::centredLeft);
	g.drawRoundedRectangle((float)lineSize + patternBlockOffsetX, (float)labelOffset + 10, (float)patternButtonOffsetX + buttonW -73, (float)patternTH + (2 * lineSize + 5), (float)4, (float)lineSize);

	patternsTable.setBounds(patternBlockOffsetX + 10, 30, patternTW - 5, patternTH);
	insertPatternButton.setBounds(patternButtonOffsetX, 40, buttonW, buttonH);
	newPatternButton.setBounds(patternButtonOffsetX, 70, buttonW, buttonH);
	deletePatternButton.setBounds(patternButtonOffsetX, 100, buttonW, buttonH);
	duplicatePatternButton.setBounds(patternButtonOffsetX, 130, buttonW, buttonH);
	overloadPatternButton.setBounds(patternButtonOffsetX, 160, buttonW, buttonH);

	settingComponent.setBounds(patternBlockOffsetX +400, 7, settingComponent.width, settingComponent.heigth);
	
} // paint

///////////////////////////////////////////////////////////////////////////

void TopiaryRiffzMasterComponent::resized()
{
} // resized

///////////////////////////////////////////////////////////////////////////

void TopiaryRiffzMasterComponent::actionListenerCallback(const String &message)
{
	if (message.compare(MsgLoad) == 0)
	{	
		patternsTable.setModel(riffzModel->getPatternList());
		//poolTable.setModel(riffzModel->getPoolList());
	}

	else if (message.compare(MsgPatternList) == 0)
	{
		// trigger update of pooltable & masterTable
		int remember = patternsTable.getSelectedRow();
		patternsTable.updateContent();

		if ((remember <= patternsTable.getNumRows()) && (remember >= 0))
		{
			patternsTable.selectRow(0);  // needed otherwise the number of measures is not correctly updated (strange)
			patternsTable.selectRow(remember);
		}
		else
			patternsTable.selectRow(0);
		setButtonStates();
	}

	if (message.compare(MsgTransport) == 0)
		getSettings();

} // actionListenerCallback

///////////////////////////////////////////////////////////////////////////

void TopiaryRiffzMasterComponent::setSettings()
{
	// called when settings changes

	riffzModel->setWFFN(settingComponent.WFFNButton.getToggleState());
	riffzModel->setNotePassThrough(settingComponent.notePassThroughButton.getToggleState());
	riffzModel->setVariationStartQ(settingComponent.quantizeVariationStartCombo.getSelectedId());
	riffzModel->setRunStopQ(settingComponent.quantizeRunStopCombo.getSelectedId());
	riffzModel->setNoteOrder(settingComponent.noteOrderCombo.getSelectedId());
	riffzModel->setName(settingComponent.nameEditor.getText());
	riffzModel->setSwitchVariation(settingComponent.switchVariationCombo.getSelectedId());
	riffzModel->setLatch(settingComponent.latchButton.getToggleState());
	// validate edits to note range
	int from = 0; 
	int to = 127;
	int oldFrom, oldTo;

	riffzModel->getKeyRange(oldFrom, oldTo);

	String note=validateNote(settingComponent.keyRangeFromEditor.getText());
	from = validNoteNumber(note);
	note = validateNote(settingComponent.keyRangeToEditor.getText());
	to = validNoteNumber(note);

	if (from > to)
	{
		riffzModel->Log("Inconsistent note range", Topiary::LogType::Warning);
		settingComponent.keyRangeFromEditor.setText(MidiMessage::getMidiNoteName(oldFrom, true, true, 5), dontSendNotification);
		settingComponent.keyRangeToEditor.setText(MidiMessage::getMidiNoteName(oldTo, true, true, 5), dontSendNotification);
	}
	else
			riffzModel->setKeyRange(from, to);

	
	// validate channel edits
	int oldChannel = riffzModel->getOutputChannel();
	int ch = settingComponent.outputChannelEditor.getText().getIntValue();
		
	if (ch < 1)
	{
		ch = oldChannel;
		riffzModel->Log("Output channel should be > 0.", Topiary::LogType::Warning);
		settingComponent.outputChannelEditor.setText(String(oldChannel));
	}
	else if (ch > 16)
	{
		ch = oldChannel;
		riffzModel->Log("Output channel should be < 17.", Topiary::LogType::Warning);
		settingComponent.outputChannelEditor.setText(String(oldChannel));
	}
	else
		riffzModel->setOutputChannel(ch);

} // setSettings

//////////////////////////////////////////////////////

void TopiaryRiffzMasterComponent::loadPreset()
{
	riffzModel->loadPreset("Please select Topiary Riffz file to load...", "*.tri");

	actionListenerCallback(MsgLoad);
	getSettings();
	patternsTable.updateContent();
	
	setButtonStates();

} // loadPreset

//////////////////////////////////////////////////////

void TopiaryRiffzMasterComponent::savePreset()
{
	riffzModel->savePreset("Please select Topiay Riffz file to load...", "*.tri");

} // savePreset

//////////////////////////////////////////////////////