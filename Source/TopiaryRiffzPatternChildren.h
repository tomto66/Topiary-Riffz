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

class TopiaryRiffzPatternComponent;

////////////////////////////////////////////////////////////////////////////////
// PatternLenghComponent
// Editor to set/edit pattern length
////////////////////////////////////////////////////////////////////////////////

class PatternLengthComponent : public Component
{

public:

	TextEditor measureEditor, beatEditor, tickEditor;
	ToggleButton keepTail; // when shortening; keep the tail
	TextButton goButton;

	PatternLengthComponent();
	~PatternLengthComponent();
	void resized();
	void paint(Graphics& g) override;
	void setParent(TopiaryRiffzPatternComponent* p);
	int width = 240;
	int heigth = 48;

private:
	TopiaryRiffzPatternComponent* parent;
	static const int eW = 30; //  size
	static const int eH = 20;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternLengthComponent)
};

////////////////////////////////////////////////////////////////////////////////
// ActionButtonsComponent
// Lots of action buttons
////////////////////////////////////////////////////////////////////////////////

class ActionButtonsComponent : public Component
{

public:
	
	ActionButtonsComponent();
	~ActionButtonsComponent();
	void resized();
	void paint(Graphics& g) override;
	void setParent(TopiaryRiffzPatternComponent* p);
	int width = 240;
	int heigth = 240;
	TextButton deleteButton;	// deletes currently selected; disabled if nothing selected
	TextButton addButton;		// adds at current selection; adds at 0 0 0 if nothing selected
	TextButton copyButton;		// copies selected note
	TextButton pasteButton;		// pastes at current selection; disabled when nothing in paste buffer
	TextButton clearButton;		// clears the pattern
	TextButton deleteAllNotesButton;  // deletes all notes in the pattern that are same as selected one; disabled if nothing selected
	TextButton quantizeButton;
	ComboBox quantizeCombo;

private:
	TopiaryRiffzPatternComponent* parent;

	static const int bW = 230; //  size
	static const int bH = 20;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ActionButtonsComponent)
};