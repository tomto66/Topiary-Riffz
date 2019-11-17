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


	////////////////////////////////////////
	// Pool stuff
	////////////////////////////////////////

	// Table for pool notes
	poolTable.setSize(poolTW, poolTH);
	addAndMakeVisible(poolTable);

	// Delete Note button
	deletePoolButton.setSize(buttonW, buttonH);
	addAndMakeVisible(deletePoolButton);
	deletePoolButton.setButtonText("Delete note");
	deletePoolButton.onClick = [this] {

		auto selection = poolTable.getSelectedRow();
		jassert(selection > -1);
		
		riffzModel->deletePoolNote(selection);
		riffzModel->generateAllVariations(-1);
		riffzModel->sendActionMessage(MsgNotePool); // tables resort the data!
		poolTable.updateContent();
		poolTable.selectRow(0);   // select the first row
	};


	// Add Note button
	newPoolButton.setSize(buttonW, buttonH);
	addAndMakeVisible(newPoolButton);
	newPoolButton.setButtonText("Add note");
	newPoolButton.onClick = [this] {
		riffzModel->addPoolNote();
		//riffzModel->sendActionMessage(MsgNotePool); // done in the model!
		poolTable.selectRow(riffzModel->getNumPatterns()-1 );   // select the new row
	};

	// Regenerate Button
	regeneratePoolButton.setSize(buttonW, buttonH);
	addAndMakeVisible(regeneratePoolButton);
	regeneratePoolButton.setButtonText("Rebuild");
	regeneratePoolButton.onClick = [this] {
		riffzModel->rebuildPool(false);
		riffzModel->generateAllVariations(-1);
		riffzModel->sendActionMessage(MsgNotePool); // tables resort the data!
	};

	GMDrumMapButton.setSize(buttonW, buttonH);
	GMDrumMapButton.setButtonText("GM Drum Map");
	GMDrumMapButton.onClick = [this] {
		riffzModel->setGMDrumMapLabels();
		poolTable.updateContent();
	};
	addAndMakeVisible(GMDrumMapButton);

	// Clean Pool Button
	cleanPoolButton.setSize(buttonW, buttonH);
	addAndMakeVisible(cleanPoolButton);
	cleanPoolButton.setButtonText("Clean");
	cleanPoolButton.onClick = [this] {
		riffzModel->rebuildPool(true); // true makes it clean!
		riffzModel->generateAllVariations(-1);
		riffzModel->sendActionMessage(MsgNotePool); // tables resort the data!
	};
	 
	auditionButton.setClickingTogglesState(true);
	auditionButton.setSize(buttonW, buttonH);
	addAndMakeVisible(auditionButton);
	auditionButton.setButtonText("Audition");
	auditionButton.onClick = [this] {
		auto selection = poolTable.getSelectedRow();
		jassert(selection > -1);
		auto debug = poolTable.getText(2, selection);
		int noteNumber = poolTable.getText(2 ,selection).getIntValue();
		jassert(noteNumber >= 0);
		jassert(noteNumber < 128);

		if (auditionButton.getToggleState())
			riffzModel->outputNoteOn(noteNumber);
		else
			riffzModel->outputNoteOff(noteNumber);
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

	poolTable.setModel(riffzModel->getPoolList()); // because it will have to validate note data!
	riffzModel->setListener((ActionListener*)this);

	// trick to call the model and read 
	actionListenerCallback(MsgNotePool);
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

	//// Pool

	g.drawText("Note Pool", 420 + 83, 10, 200, labelOffset, juce::Justification::centredLeft);
	g.drawRoundedRectangle((float)lineSize + 420 + 83, (float)labelOffset + 10, (float)400, (float)patternTH + (2 * lineSize + 5), (float)4, (float)lineSize);
	poolTable.setBounds(435 + 83, 30, poolTW - 5, poolTH);

	int cleanButtonOffsetX = patternButtonOffsetX + 415;

	cleanPoolButton.setBounds(cleanButtonOffsetX , 40, buttonW, buttonH);
	newPoolButton.setBounds(cleanButtonOffsetX , 70, buttonW, buttonH);
	deletePoolButton.setBounds(cleanButtonOffsetX , 100, buttonW, buttonH);
	GMDrumMapButton.setBounds(cleanButtonOffsetX , 130, buttonW, buttonH);
	regeneratePoolButton.setBounds(cleanButtonOffsetX , 160, buttonW, buttonH);
	auditionButton.setBounds(cleanButtonOffsetX, 190, buttonW, buttonH);
	settingComponent.setBounds(patternBlockOffsetX, 280, settingComponent.width, settingComponent.heigth);
	
} // paint

void TopiaryRiffzMasterComponent::resized()
{
} // resized

///////////////////////////////////////////////////////////////////////////

void TopiaryRiffzMasterComponent::actionListenerCallback(const String &message)
{
	if (message.compare(MsgLoad) == 0)
	{	
		patternsTable.setModel(riffzModel->getPatternList());
		poolTable.setModel(riffzModel->getPoolList());
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
	else if (message.compare(MsgNotePool) == 0)
	{
		poolTable.updateContent();
		poolTable.selectRow(0);
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
	riffzModel->setName(settingComponent.nameEditor.getText());
	riffzModel->setSwitchVariation(settingComponent.switchVariationCombo.getSelectedId());
	riffzModel->setFixedOutputChannels(settingComponent.forceOutputChannelButton.getToggleState());
} // setSettings

//////////////////////////////////////////////////////

void TopiaryRiffzMasterComponent::loadPreset()
{
	riffzModel->loadPreset("Please select Topiary Riffz file to load...", "*.tbe");

	actionListenerCallback(MsgLoad);
	getSettings();
	patternsTable.updateContent();
	poolTable.updateContent();
	setButtonStates();

} // loadPreset

//////////////////////////////////////////////////////

void TopiaryRiffzMasterComponent::savePreset()
{
	riffzModel->savePreset("Please select Topiay Riffz file to load...", "*.tbe");

} // savePreset

//////////////////////////////////////////////////////