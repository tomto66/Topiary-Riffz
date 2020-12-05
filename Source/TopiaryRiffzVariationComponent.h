/////////////////////////////////////////////////////////////////////////////
/*
This file is part of Topiary Riffz, Copyright Tom Tollenaere 2019-21.

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
#include "TopiaryRiffzVariationChildren.h"

class TopiaryRiffzVariationComponent : public Component, ActionListener
{
public:
	TopiaryRiffzVariationComponent();
	~TopiaryRiffzVariationComponent();

	void paint(Graphics&) override;
	void resized() override;
	void setModel(TopiaryRiffzModel* m);
	void setVariationDefinition();  // write to model
	void getVariationDefinition();  // get data from model
	
	void getRandomizeNotes();
	void setRandomizeNotes();
	void setNoteLength();
	void getNoteLength();
	void setSwing();
	void getSwing();
	void setVelocity();
	void getVelocity();
	void getTiming();
	void setTiming();
	void getNoteAssignment();
	void setNoteAssignment();

	void setSwingQ();
	void getSwingQ();

	void actionListenerCallback(const String &message) override;
	TopiaryRiffzModel* riffzModel;
	bool newButtonClicked = false;

	void saveNoteAssignment();
	void deleteNoteAssignment();

	void enable();
	void disable();

private:
	   
	
	VariationDefinitionComponent variationDefinitionComponent;	
	VariationTypeComponent variationTypeComponent;
	RandomNoteComponent randomNoteComponent;
	NoteLengthComponent noteLengthComponent;
	SwingComponent swingComponent;
	VelocityComponent velocityComponent;
	TimingComponent timingComponent;	
	SwingQComponent swingQComponent;
	NoteAssignmentComponent noteAssignmentComponent;
	

	int variation; // the one we are working on; 0-7
	
	bool initializing = true; // to avoid unneeded warnings @init. Set to true when get/set detects numPatterns = 0; set to false as soons as a patternId of 1 gets written.
	bool getVariationCalledFromChangeInVariationButton = false;

	//////////////////////////////////////////////////////

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopiaryRiffzVariationComponent)
};
