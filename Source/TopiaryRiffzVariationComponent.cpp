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

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../Topiary/Source/Topiary.h"
#include "TopiaryRiffzModel.h"
#include "TopiaryRiffzVariationComponent.h"


TopiaryRiffzVariationComponent::TopiaryRiffzVariationComponent()
{   // size set by paint!
	variation = 0;

	variationDefinitionComponent.setParent(this);
	addAndMakeVisible(variationDefinitionComponent);
	
	randomNoteComponent.setParent(this);
	addAndMakeVisible(randomNoteComponent);

	addAndMakeVisible(noteLengthComponent);
	noteLengthComponent.setParent(this);

	addAndMakeVisible(swingComponent);
	swingComponent.setParent(this);

	addAndMakeVisible(velocityComponent);
	velocityComponent.setParent(this);

	addAndMakeVisible(timingComponent);
	timingComponent.setParent(this);
	
	addAndMakeVisible(variationTypeComponent);
	variationTypeComponent.setParent(this);

	noteLengthComponent.setParent(this);
	addAndMakeVisible(noteLengthComponent);

	addAndMakeVisible(swingQComponent);
	swingQComponent.setParent(this);

	addAndMakeVisible(noteAssignmentComponent);
	//noteAssignmentComponent.setParent(this);  don't do this here - do this in setModel - otherwise we get a dangling pointer!!!

} // TopiaryRiffzVariationComponent() 

/////////////////////////////////////////////////////////////////////////

TopiaryRiffzVariationComponent::~TopiaryRiffzVariationComponent()
{
	riffzModel->removeListener((ActionListener*)this);

} // ~TopiaryRiffzVariationComponent

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::setModel(TopiaryRiffzModel* m)
{
	riffzModel = m;	
	noteAssignmentComponent.setParent(this);   // not done earlier because noteAssignmentCompentent needs riffzModel to initiate noteAssignmentTable !!!

	riffzModel->setListener((ActionListener*)this);
	actionListenerCallback(MsgPatternList); // need to call this so we can fill the patternCombo !!!
	
	variationDefinitionComponent.variationCombo.setSelectedId(1);  // this will trigger a call to getVariationDefinition which gets the data
	actionListenerCallback(MsgPatternList); // fill the pattern list

	noteAssignmentComponent.noteEditor.setModel(riffzModel, Topiary::LearnMidiId::noteAssignment);
	
	actionListenerCallback(MsgSelectedNoteAssignmentRowsChanged); // set some buttons right on noteAssignmentComponent

} // setModel

/////////////////////////////////////////////////////////////////////////


void TopiaryRiffzVariationComponent::setVariationDefinition()
{
	// write to model
	if (riffzModel->getNumPatterns() == 0)
	{
		initializing = true; // don't do anything
		return;
	}
	
	if ((riffzModel->getNumPatterns() == 1) && (initializing==true))
	{
		initializing = false; // don't do anything but un-set initializing
	}
	
	

	if (variation != (variationDefinitionComponent.variationCombo.getSelectedId() - 1))
	{
		variation = variationDefinitionComponent.variationCombo.getSelectedId() - 1;
	}

	jassert(variation > -1);

	bool refused = riffzModel->validateVariationDefinition(variation, variationDefinitionComponent.nameEditor.getText());

	if (!refused)
	{
		riffzModel->setVariationDefinition(variation, variationDefinitionComponent.enableButton.getToggleState(), variationDefinitionComponent.nameEditor.getText(), variationTypeComponent.type);
		riffzModel->Log("Variation "+String(variation+1)+" saved.", Topiary::LogType::Info);
	}
	else
		variationDefinitionComponent.enableButton.setToggleState(false, dontSendNotification);

	setRandomizeNotes();
	setSwing();
	setVelocity();
	setTiming();
	setSwingQ();
	setNoteLength();
	initializing = false;

}  // setVariationDefinition

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::getVariationDefinition()
{
	// get from model

	variation = variationDefinitionComponent.variationCombo.getSelectedId() - 1;
	if (variation != -1 && !getVariationCalledFromChangeInVariationButton)  // because otherwise the variation is already set in the editor
			riffzModel->setVariation(variation); // so that the variation buttons follow

		// make sure the noteassignment table is linked to the variation selected
	noteAssignmentComponent.noteAssignmentTable.setModel(riffzModel->getNoteAssignment(variation));

	if (variation <0 ) return;  // this should never happen, except when initializing

	bool enable;
	int type;
	String vname;
	

	riffzModel->getVariationDefinition(variation, enable, vname,  type);
	variationDefinitionComponent.enableButton.setToggleState(enable, dontSendNotification);
	variationDefinitionComponent.nameEditor.setText(vname, dontSendNotification);
	
	switch (type) 
	{
	case (Topiary::VariationTypeSteady):
			variationTypeComponent.steadyButton.setToggleState(true, dontSendNotification);
			break;
	case (Topiary::VariationTypeEnd):
		variationTypeComponent.endButton.setToggleState(true, dontSendNotification);
		break;
	case (Topiary::VariationTypeFill):
		variationTypeComponent.fillButton.setToggleState(true, dontSendNotification);
		break;
	case (Topiary::VariationTypeIntro):
		variationTypeComponent.introButton.setToggleState(true, dontSendNotification);
		break;
	}
	
	getRandomizeNotes();
	getSwing();
	getVelocity();
	getTiming();
	getSwingQ();
	getNoteLength();

} // getVariationDefinition

/////////////////////////////////////////////////////////////////////////
/*
void TopiaryRiffzVariationComponent::getNoteLength()
{
	// get note randomisation from model
	bool enable;
	
	int value;

	riffzModel->getRandomizeNotes(variationDefinitionComponent.variationCombo.getSelectedId()-1, enable, value);

	randomNoteComponent.onButton.setToggleState(enable, dontSendNotification);
	
	randomNoteComponent.percentSlider.setValue(value, dontSendNotification);

} // getRandomizeNotes
*/
/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::getRandomizeNotes()
{
	// get note randomisation from model
	bool enable;
	int value;

	riffzModel->getRandomizeNotes(variationDefinitionComponent.variationCombo.getSelectedId() - 1, enable, value);

	randomNoteComponent.onButton.setToggleState(enable, dontSendNotification);
	
	randomNoteComponent.percentSlider.setValue(value, dontSendNotification);

} // getRandomizeNotes

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::setRandomizeNotes()
{
	// set note randomisation to model


	riffzModel->setRandomizeNotes(variationDefinitionComponent.variationCombo.getSelectedId() - 1, randomNoteComponent.onButton.getToggleState(), (int)randomNoteComponent.percentSlider.getValue());

} // setRandomizeNotes

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::setNoteLength()
{
	// set note randomisation to model

	riffzModel->setRandomizeLength(variationDefinitionComponent.variationCombo.getSelectedId()-1, 
									noteLengthComponent.onButton.getToggleState(), 
									(int) noteLengthComponent.percentSlider.getValue(),
									noteLengthComponent.plusButton.getToggleState(),
									noteLengthComponent.minusButton.getToggleState());

} // setNoteLength

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::getNoteLength()
{
	int v, value;
	bool enable, plus, min;

	v = 0;

	riffzModel->getRandomizeLength(v, enable, value, plus, min);
	
	noteLengthComponent.onButton.setToggleState(enable, dontSendNotification),
	noteLengthComponent.percentSlider.setValue(value, dontSendNotification),
	noteLengthComponent.plusButton.setToggleState(plus, dontSendNotification),
	noteLengthComponent.minusButton.setToggleState(min, dontSendNotification);

} // getNoteLength

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::setSwing()
{
	// set note randomisation to model

	riffzModel->setSwing(variationDefinitionComponent.variationCombo.getSelectedId() - 1, swingComponent.onButton.getToggleState(), (int)swingComponent.swingSlider.getValue());

} // setSwing

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::getSwing()
{
	// get note randomisation from model
	bool enable;
	
	int value;

	riffzModel->getSwing(variationDefinitionComponent.variationCombo.getSelectedId() - 1, enable, value);

	swingComponent.onButton.setToggleState(enable, dontSendNotification);
	
	swingComponent.swingSlider.setValue(value, dontSendNotification);

} // getSwing

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::setTiming()
{
	// set note randomisation to model

	riffzModel->setRandomizeTiming(variationDefinitionComponent.variationCombo.getSelectedId() - 1, timingComponent.onButton.getToggleState(),  
									(int)timingComponent.timingSlider.getValue(),
									timingComponent.plusButton.getToggleState(),
									timingComponent.minusButton.getToggleState());

} // setTiming

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::getTiming()
{
	// get note randomisation from model
	bool enable;
	bool plus, min;
	int value;

	riffzModel->getRandomizeTiming(variationDefinitionComponent.variationCombo.getSelectedId() - 1, enable, value, plus, min);

	timingComponent.onButton.setToggleState(enable, dontSendNotification);
	
	timingComponent.timingSlider.setValue(value, dontSendNotification);
	timingComponent.plusButton.setToggleState(plus, dontSendNotification);
	timingComponent.minusButton.setToggleState(min, dontSendNotification);

} // getTiming


/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::setVelocity()
{
	// set note randomisation to model

	riffzModel->setRandomizeVelocity(variationDefinitionComponent.variationCombo.getSelectedId() - 1, velocityComponent.onButton.getToggleState(),  
									(int)velocityComponent.velocitySlider.getValue(),
									velocityComponent.plusButton.getToggleState(),
									velocityComponent.minusButton.getToggleState() 	);

} // setVelocity

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::getVelocity()
{
	// get note randomisation from model
	bool enable;
	bool plus, min;
	int value;

	riffzModel->getRandomizeVelocity (variationDefinitionComponent.variationCombo.getSelectedId() - 1, enable, value, plus, min);

	velocityComponent.onButton.setToggleState(enable, dontSendNotification);
	
	velocityComponent.velocitySlider.setValue(value, dontSendNotification);
	velocityComponent.plusButton.setToggleState(plus, dontSendNotification);
	velocityComponent.minusButton.setToggleState(min, dontSendNotification);

} // getVelocity


/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::setSwingQ()
{
	riffzModel->setSwingQ(variationDefinitionComponent.variationCombo.getSelectedId() - 1, swingQComponent.swingQ);

} //setSwingQ

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::getSwingQ()
{
	int swingQ = riffzModel->getSwingQ(variationDefinitionComponent.variationCombo.getSelectedId() - 1);

	if (swingQ == TopiaryRiffzModel::SwingQButtonIds::SwingQ8)
		swingQComponent.eightButton.setToggleState(true, dontSendNotification);
	else
		swingQComponent.fourButton.setToggleState(true, dontSendNotification);

} //setSwingQ

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::getNoteAssignment()
{

}

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::setNoteAssignment()
{
	// if nothing selected in the table, save as a new one
	// if something selected, overwrite the assignment
	jassert(false);
}

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::actionListenerCallback(const String &message)
{
	if (message.compare(MsgVariationDefinition) == 0)
	{
		getVariationDefinition();  
		// be sure that the mastertables are read first so the patternCombo is set correctly !!!
	}
	else if (message.compare(MsgPatternList) == 0)
	{
		// fill the patternCombo; careful, there may already be stuff there so clear it and then set it back where it was
		int remember = noteAssignmentComponent.patternCombo.getSelectedId();

		noteAssignmentComponent.patternCombo.clear();
		int numPatterns = riffzModel->getNumPatterns();

		String patterns[MAXNOPATTERNS];
		for (int i = 0; i < numPatterns; i++) patterns[i] = "";

		riffzModel->getPatterns(patterns);
		
		for (int i=0; i<numPatterns; i++)
		{
			if (patterns[i].compare(""))
				noteAssignmentComponent.patternCombo.addItem(patterns[i], i+1);		
		} 

		// reinitialize the notassignemnttable, because it may have labels of patterns that got renamed
		noteAssignmentComponent.noteAssignmentTable.updateContent();
	
		// if there are no patterns, disable yourself!!!
		if (riffzModel->getNumPatterns()>0)
			this->setEnabled(true);
		else
			this->setEnabled(false);

		if (remember <= noteAssignmentComponent.patternCombo.getNumItems())
			noteAssignmentComponent.patternCombo.setSelectedId(remember, dontSendNotification);
		else
			noteAssignmentComponent.patternCombo.setSelectedId(0, dontSendNotification);
	}
	else if (message.compare(MsgVariationSelected) == 0)
	{
		// set the combo to the selected variation
		int unused, newVariation;
		riffzModel->getVariation(newVariation, unused);
		if (variation != newVariation)
		{
			variationDefinitionComponent.variationCombo.setSelectedId(newVariation + 1, dontSendNotification);
			getVariationCalledFromChangeInVariationButton = true;
			getVariationDefinition();
			getVariationCalledFromChangeInVariationButton = false;
			UNUSED(unused)
		}
	}
	else if (message.compare(MsgNoteAssignment) == 0)
	{
		// update the note assignment table because note assignment got saved (among other conditions)
		noteAssignmentComponent.noteAssignmentTable.updateContent();
	}
	else if (message.compare(MsgSelectedNoteAssignmentRowsChanged) == 0)
	{
		// check if something selected or not
		int selected = noteAssignmentComponent.noteAssignmentTable.getSelectedRow();
		if (selected >= 0)
			// if selected, reflect in the righthand controls
		{
			int o, patternId;
			String n;
			riffzModel->getNoteAssignment(variation, selected, n, o, patternId);
			noteAssignmentComponent.noteEditor.setText(n, dontSendNotification);
			noteAssignmentComponent.offsetEditor.setText(String(o), dontSendNotification);
			noteAssignmentComponent.patternCombo.setSelectedId(patternId+1, dontSendNotification);
			noteAssignmentComponent.noteEditor.setEnabled(true);
			noteAssignmentComponent.offsetEditor.setEnabled(true);
			noteAssignmentComponent.patternCombo.setEnabled(true); 
			noteAssignmentComponent.deleteButton.setEnabled(true);
			noteAssignmentComponent.saveButton.setEnabled(true);
		}
		else
		{
			// clear the righthand controls - disable them if New not hit
			noteAssignmentComponent.noteEditor.setText("", dontSendNotification);
			noteAssignmentComponent.offsetEditor.setText("", dontSendNotification);
			noteAssignmentComponent.patternCombo.setSelectedId(-1, dontSendNotification);
			if (newButtonClicked)
			{
				noteAssignmentComponent.noteEditor.setEnabled(true);
				noteAssignmentComponent.offsetEditor.setEnabled(true);
				noteAssignmentComponent.patternCombo.setEnabled(true);
				noteAssignmentComponent.deleteButton.setEnabled(false);
				noteAssignmentComponent.saveButton.setEnabled(true);

			}
			else
			{
				noteAssignmentComponent.noteEditor.setEnabled(false);
				noteAssignmentComponent.offsetEditor.setEnabled(false);
				noteAssignmentComponent.patternCombo.setEnabled(false);
				noteAssignmentComponent.deleteButton.setEnabled(false);
				noteAssignmentComponent.saveButton.setEnabled(false);

			}
		}
	}

	
}  // actionListenerCallback

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::paint(Graphics& g)
{
	UNUSED(g)
	
	variationDefinitionComponent.setBounds(50, 30, variationDefinitionComponent.width, variationDefinitionComponent.heigth);
	variationTypeComponent.setBounds(50, 150, variationTypeComponent.width, variationTypeComponent.heigth);
	
	
	randomNoteComponent.setBounds(210, 30, randomNoteComponent.width, randomNoteComponent.heigth);
	swingComponent.setBounds(315, 30, swingComponent.width, swingComponent.heigth);
	velocityComponent.setBounds(420, 30, velocityComponent.width, velocityComponent.heigth);
	timingComponent.setBounds(315, 150, timingComponent.width, timingComponent.heigth);
	
	noteLengthComponent.setBounds(210, 150, noteLengthComponent.width, noteLengthComponent.heigth);

	
	swingQComponent.setBounds(210, 270, swingQComponent.width, swingQComponent.heigth);

	noteAssignmentComponent.setBounds(525, 30, noteAssignmentComponent.width, noteAssignmentComponent.heigth);

} // paint

//////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::resized()
{
} // resized

//////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::saveNoteAssignment()
{
	// make sure note is valid
	if (noteAssignmentComponent.noteEditor.getText().compare("")==0)
	{
		riffzModel->Log("No note selected.", Topiary::LogType::Warning);
		return;
	}

	String validatedText = validateNote(noteAssignmentComponent.noteEditor.getText());  
	String debug = noteAssignmentComponent.noteEditor.getText();
	if (validatedText.compare(noteAssignmentComponent.noteEditor.getText().toUpperCase())!=0)		
	// invalid note
	{
		riffzModel->Log("Invalid note.", Topiary::LogType::Warning);
		return;
	}

	noteAssignmentComponent.noteEditor.setText(validatedText, dontSendNotification);

	int notenumber = validNoteNumber(validatedText);
	
	// make sure offset is integer and note = offset is valid
	if (noteAssignmentComponent.offsetEditor.getText().compare("") == 0)
		noteAssignmentComponent.offsetEditor.setText("0", dontSendNotification);
	int offset = noteAssignmentComponent.offsetEditor.getText().getIntValue();

	if (((notenumber + offset) < 0) || ((notenumber + offset) > 127))
	{
		riffzModel->Log("Combination of note and offset invalid", Topiary::LogType::Warning);
		return;
	}

	// make sure pattern is selected
	if (noteAssignmentComponent.patternCombo.getSelectedItemIndex() < 0)
	{
		riffzModel->Log("No pattern selected", Topiary::LogType::Warning);
		return;
	}

	riffzModel->saveNoteAssignment(variation, notenumber, offset, noteAssignmentComponent.patternCombo.getSelectedItemIndex());
	noteAssignmentComponent.noteAssignmentTable.updateContent();
	// make sure something is selected in the table
	noteAssignmentComponent.noteAssignmentTable.selectRow(-1);// otherwise if row 0 is selected and edited, the edit does not show
	noteAssignmentComponent.noteAssignmentTable.selectRow(0);

} // saveNoteAssignment

//////////////////////////////////////////////////////////////////////////

void TopiaryRiffzVariationComponent::deleteNoteAssignment()
{
	// deletes the selected note assignment
	int selected = noteAssignmentComponent.noteAssignmentTable.getSelectedRow();
	if (selected >= 0)
	{
		riffzModel->deleteNoteAssignment(variation, selected);
		noteAssignmentComponent.noteAssignmentTable.updateContent();
		// make sure something is selected in the table
		noteAssignmentComponent.noteAssignmentTable.selectRow(-1);// otherwise if row 0 is selected and edited, the edit does not show
		noteAssignmentComponent.noteAssignmentTable.selectRow(0);
	}
	

}