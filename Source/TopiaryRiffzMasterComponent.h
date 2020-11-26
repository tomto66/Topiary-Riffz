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
#include "TopiaryRiffzMasterChildren.h"
#include "../../Topiary/Source/Components/TopiaryTable.h"

class TopiaryRiffzMasterComponent : public Component, ActionListener
{
public:
	TopiaryRiffzMasterComponent();
	~TopiaryRiffzMasterComponent();

	void paint(Graphics&) override;
	void resized() override;
	void setModel(TopiaryRiffzModel* m);
	void setSettings();
	void loadPreset();
	void savePreset();
	TopiaryRiffzModel* riffzModel;

private:
	
	TopiaryLookAndFeel topiaryLookAndFeel;
	void actionListenerCallback(const String &message);

	// patterns stuff

	TopiaryTable patternsTable;
	XmlElement *patternListHeader = nullptr;
	XmlElement *patternListData = nullptr;
	TextButton insertPatternButton;
	TextButton duplicatePatternButton;
	TextButton deletePatternButton;
	TextButton newPatternButton;
	TextButton overloadPatternButton;

	int buttonH = 20;
	int buttonW = 100;

	int patternTW = 253;
	int patternTH = 240;

		// settings stuff
	SettingComponent settingComponent;

	//////////////////////////////////////////////////////

	void setButtonStates() {
		// check which buttons to disable/enable
		// if no data in table, then delete is disbled and load/insert data disabled
		if (patternsTable.getNumRows() == 0) {
			insertPatternButton.setEnabled(false);
			duplicatePatternButton.setEnabled(false);
			deletePatternButton.setEnabled(false);
			overloadPatternButton.setEnabled(false);
		}
		else
		{
			if (patternsTable.getSelectedRow() < 0)
			{
				 // nothing selected
				insertPatternButton.setEnabled(false);
				duplicatePatternButton.setEnabled(false);
				deletePatternButton.setEnabled(false);
				overloadPatternButton.setEnabled(false);
			}
			else
			{
				insertPatternButton.setEnabled(true);
				duplicatePatternButton.setEnabled(true);
				deletePatternButton.setEnabled(true);	
				overloadPatternButton.setEnabled(true);
			}
				
		}
		
		if (patternsTable.getNumRows() == MAXNOPATTERNS)
			newPatternButton.setEnabled(false);
		else newPatternButton.setEnabled(true);
		
	}

	//////////////////////////////////////////////////////

	void getSettings()
	{  
		// called when settings change
		
		settingComponent.WFFNButton.setToggleState(riffzModel->getWFFN(), dontSendNotification);
		settingComponent.notePassThroughButton.setToggleState(riffzModel->getNotePassThrough(), dontSendNotification);
		settingComponent.quantizeVariationStartCombo.setSelectedId(riffzModel->getVariationStartQ(), dontSendNotification);
		settingComponent.quantizeRunStopCombo.setSelectedId(riffzModel->getRunStopQ(), dontSendNotification);
		settingComponent.switchVariationCombo.setSelectedId(riffzModel->getSwitchVariation(), dontSendNotification);
		settingComponent.noteOrderCombo.setSelectedId(riffzModel->getNoteOrder(), dontSendNotification);
		bool l1, l2;
		riffzModel->getLatch(l1, l2);
		settingComponent.latch1Button.setToggleState(l1, dontSendNotification);
		settingComponent.latch2Button.setToggleState(l2, dontSendNotification);
		settingComponent.outputChannelEditor.setText(String(riffzModel->getOutputChannel()), dontSendNotification);

		int from, to;
		riffzModel->getKeyRange(from, to);
		settingComponent.keyRangeFromEditor.setText(MidiMessage::getMidiNoteName(from, true, true, 5), dontSendNotification);
		settingComponent.keyRangeToEditor.setText(MidiMessage::getMidiNoteName(to, true, true, 5), dontSendNotification);
		

		auto name = riffzModel->getName();
		settingComponent.nameEditor.setText(name, dontSendNotification);
		
		// disable when host overridden
		int b, n, d, bs;
		bool override, wf;
		riffzModel->getTransportState(b, n, d, bs, override, wf);
		
	} // getSettings
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopiaryRiffzMasterComponent)
};

/////////////////////////////////////////////////////////////////////


