/////////////////////////////////////////////////////////////////////////////
/*
This file is part of Topiary Beatz, Copyright Tom Tollenaere 2018-20.

Topiary Beats is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Topiary Beats is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Topiary Beats. If not, see <https://www.gnu.org/licenses/>.
*/
/////////////////////////////////////////////////////////////////////////////

#include "TopiaryRiffzModel.h"
#include "TopiaryRiffzVariationComponent.h"
#include "TopiaryRiffzVariationChildren.h"
#include "../../Topiary/Source/Components/TopiaryVariationChildren.cpp.h"

/////////////////////////////////////////////////////////////////////////
// NoteAssignmentComponent
/////////////////////////////////////////////////////////////////////////

NoteAssignmentComponent::NoteAssignmentComponent()
{

	// Table for patterns
	noteAssignmentTable.setSize(tW, tH);
	addAndMakeVisible(noteAssignmentTable);

	newButton.setButtonText("New");
	addAndMakeVisible(newButton);
	newButton.setSize(buttonH, buttonW);
	newButton.onClick = [this]
	{
		noteAssignmentTable.selectRow(-1);
		offsetEditor.setText("");
		noteEditor.setText("");
		patternCombo.setSelectedItemIndex(0);
		noteEditor.setEnabled(true);
		offsetEditor.setEnabled(true);
		patternCombo.setEnabled(true);
		parent->newButtonClicked = true;
		saveButton.setEnabled(true);
		noteAssignmentTable.selectRow(-1);
	};

		

	deleteButton.setButtonText("Delete");
	addAndMakeVisible(deleteButton);
	deleteButton.setSize(buttonW, buttonH);
	deleteButton.onClick = [this]
	{
		parent->deleteNoteAssignment();
		parent->newButtonClicked = false;
	};

	saveButton.setButtonText("Save");
	addAndMakeVisible(saveButton);
	saveButton.setSize(buttonW, buttonH);
	saveButton.onClick = [this]
	{
		parent->saveNoteAssignment();
		parent->newButtonClicked = false;
	};

	addAndMakeVisible(patternCombo);
	patternCombo.setSize(buttonW, buttonH);
	patternCombo.onChange = [this]
	{
		
	};


	addAndMakeVisible(offsetEditor);
	offsetEditor.setSize(editorW, editorH);
	offsetEditor.onReturnKey = [this]
	{
		
	};


	addAndMakeVisible(noteEditor);
	noteEditor.setSize(editorW, editorH);
	noteEditor.onReturnKey = [this]
	{
		String validatedText = validateNote(noteEditor.getText());
		noteEditor.setText(validatedText, dontSendNotification);
	};
	noteEditor.onFocusLost = [this]
	{
		String validatedText = validateNote(noteEditor.getText());
		noteEditor.setText(validatedText, dontSendNotification);
	};

	
} // NoteAssignmentComponent

///////////////////////////////////////////////////////////////////////////////////

NoteAssignmentComponent::~NoteAssignmentComponent()
{
} // ~NoteAssignmentComponent

///////////////////////////////////////////////////////////////////////////////////

void NoteAssignmentComponent::setParent(TopiaryRiffzVariationComponent* p)
{
	parent = p;
	noteAssignmentTable.setModel(p->riffzModel->getNoteAssignment(0));
} // setParent

  ///////////////////////////////////////////////////////////////////////////////////

void  NoteAssignmentComponent::resized()
{

} // resized

///////////////////////////////////////////////////////////////////////////////////

void  NoteAssignmentComponent::paint(Graphics& g)
{

	int labelOffset = 15;
	int lineWidth = 2;
	int spacer = 10;
	auto area = getLocalBounds();
	g.fillAll(TopiaryColour::background);
	g.setColour(TopiaryColour::foreground);
	g.setFont(12.0f);

	g.drawText("Note Assignment", (int)lineWidth, (int)lineWidth, 200, (int)labelOffset, juce::Justification::centredLeft);
	auto recBounds = area.removeFromBottom(area.getHeight() - labelOffset);
	g.drawRoundedRectangle((float)recBounds.getX() + lineWidth, (float)recBounds.getY() + lineWidth, (float)recBounds.getWidth() - 2 * lineWidth, (float)recBounds.getHeight() - 2 * lineWidth, (float)lineWidth, (float)lineWidth);

	// turn it into inner rectangle
	auto inRecBounds = Rectangle<int>::Rectangle(recBounds.getX() + 3 * lineWidth, recBounds.getY() + 3 * lineWidth, recBounds.getWidth() - 6 * lineWidth, recBounds.getHeight() - 6 * lineWidth);

	auto tBounds = inRecBounds.removeFromLeft(250);
	noteAssignmentTable.setBounds(tBounds);

	// inRecBounds is now column 2

	inRecBounds.removeFromTop(spacer);
	tBounds = inRecBounds.removeFromTop(buttonH);
	newButton.setBounds(tBounds);

	inRecBounds.removeFromTop(spacer);
	tBounds = inRecBounds.removeFromTop(buttonH);
	deleteButton.setBounds(tBounds);

	inRecBounds.removeFromTop(spacer);
	tBounds = inRecBounds.removeFromTop(buttonH);
	saveButton.setBounds(tBounds);

	inRecBounds.removeFromTop(spacer);
	tBounds = inRecBounds.removeFromTop(buttonH);
	patternCombo.setBounds(tBounds);

	// line for the 2 editors
	inRecBounds.removeFromTop(spacer);
	tBounds = inRecBounds.removeFromTop(editorH);

	tBounds.removeFromLeft(55);
	auto editorBounds = tBounds.removeFromLeft(editorW);
	noteEditor.setBounds(editorBounds);

	tBounds.removeFromLeft(55);
	editorBounds = tBounds.removeFromLeft(editorW);
	offsetEditor.setBounds(editorBounds);

	g.drawText("Note:                     Offset:", 260, 152, 240, labelOffset + 3, juce::Justification::centredLeft);

} // paint

// noteAssignmentComponent