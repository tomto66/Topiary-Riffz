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

class TopiaryRiffzMasterComponent;

class SettingComponent : public Component
{

public:
	TopiaryLookAndFeel topiaryLookAndFeel;
	TopiaryButton WFFNButton;
	TopiaryButton notePassThroughButton;
	ComboBox switchVariationCombo;
	ComboBox quantizeVariationStartCombo;
	ComboBox quantizeRunStopCombo;
	ComboBox noteOrderCombo;

	TextButton saveButton;
	TextButton loadButton;
	TextEditor nameEditor;
	TopiaryButton latchButton;
	TextEditor outputChannelEditor;
	TopiaryMidiLearnEditor keyRangeFromEditor, keyRangeToEditor;
	

	SettingComponent();
	~SettingComponent();
	void resized();
	void paint(Graphics& g) override;
	void setParent(TopiaryRiffzMasterComponent* p);
	int width = 380;
	int heigth = 269;

private:
	TopiaryRiffzMasterComponent* parent;
	static const int eW = 120; //  size
	static const int eH = 20;
	static const int buttonH = 20;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingComponent)
};
