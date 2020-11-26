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
////////////////////////////////////////////////////////////////////////////

#pragma once
#include "TopiaryRiffzMasterChildren.h"
#include "TopiaryRiffzMasterComponent.h"

SettingComponent::SettingComponent()
{

	setSize(width, heigth);

	// WFFNButton

	WFFNButton.setClickingTogglesState(true);
	WFFNButton.setLookAndFeel(&topiaryLookAndFeel);
	WFFNButton.setSize(100, buttonH);
	addAndMakeVisible(WFFNButton);
	WFFNButton.setButtonText("WFFN");
	WFFNButton.onClick = [this]
	{
		parent->setSettings();
	};

	// WFFNButton

	latch1Button.setClickingTogglesState(true);
	latch1Button.setLookAndFeel(&topiaryLookAndFeel);
	latch1Button.setSize(45, buttonH);
	addAndMakeVisible(latch1Button);
	latch1Button.setButtonText("Latch1");
	
	latch1Button.onClick = [this]
	{
		if (latch1Button.getToggleState())
			latch2Button.setToggleState(false, dontSendNotification);

		parent->setSettings();
	};

	latch2Button.setClickingTogglesState(true);
	latch2Button.setLookAndFeel(&topiaryLookAndFeel);
	latch2Button.setSize(45, buttonH);
	
	addAndMakeVisible(latch2Button);
	latch2Button.setButtonText("Latch2");
	latch2Button.onClick = [this]
	{
		if (latch2Button.getToggleState())
			latch1Button.setToggleState(false, dontSendNotification);

		parent->setSettings();
	};

	// notePassThoughButton

	notePassThroughButton.setClickingTogglesState(true);
	notePassThroughButton.setLookAndFeel(&topiaryLookAndFeel);
	notePassThroughButton.setSize(100, buttonH);
	addAndMakeVisible(notePassThroughButton);
	notePassThroughButton.setButtonText("Echo MIDI");
	notePassThroughButton.onClick = [this]
	{
		parent->setSettings();
	};

	// quantizeVariationStartCombo;

	quantizeVariationStartCombo.setSize(100, buttonH);
	addAndMakeVisible(quantizeVariationStartCombo);
	quantizeVariationStartCombo.setSize(100, buttonH);
	quantizeVariationStartCombo.addItem("Immediate", Topiary::Immediate);
	quantizeVariationStartCombo.addItem("Measure", Topiary::Measure);
	quantizeVariationStartCombo.addItem("Beat", Topiary::Quarter);

	quantizeVariationStartCombo.onChange = [this]
	{
		parent->setSettings();
	};

	// quantizeRunStop

	quantizeRunStopCombo.setSize(100, buttonH);
	addAndMakeVisible(quantizeRunStopCombo);
	quantizeRunStopCombo.setSize(100, buttonH);
	quantizeRunStopCombo.addItem("Immediate", 100);
	quantizeRunStopCombo.addItem("Whole variation", Topiary::WholePattern);
	quantizeRunStopCombo.addItem("Measure", 1);
	quantizeRunStopCombo.addItem("1/2", 3);
	quantizeRunStopCombo.addItem("1/4", 5);

	quantizeRunStopCombo.onChange = [this]
	{
		parent->setSettings();
	};

	// noteOrder

	noteOrderCombo.setSize(100, buttonH);
	addAndMakeVisible(noteOrderCombo);
	noteOrderCombo.setSize(100, buttonH);
	noteOrderCombo.addItem("Lowest", TopiaryKeytracker::NoteOrder::Lowest);
	noteOrderCombo.addItem("Highest", TopiaryKeytracker::NoteOrder::Highest);
	noteOrderCombo.addItem("First", TopiaryKeytracker::NoteOrder::First);
	noteOrderCombo.addItem("Last", TopiaryKeytracker::NoteOrder::Last);
	
	noteOrderCombo.onChange = [this]
	{
		parent->setSettings();
	};

	addAndMakeVisible(saveButton);
	saveButton.setSize(100, buttonH);
	saveButton.setButtonText("Save");
	saveButton.onClick = [this]
	{
		parent->savePreset();
	};

	addAndMakeVisible(loadButton);
	loadButton.setSize(100, buttonH);
	loadButton.setButtonText("Load");
	loadButton.onClick = [this]
	{
		parent->loadPreset();
	};

	addAndMakeVisible(nameEditor);
	nameEditor.setSize(210, buttonH);
	nameEditor.onReturnKey = [this]
	{
		parent->setSettings();
	};
	nameEditor.onFocusLost = [this]
	{
		parent->setSettings();
	};

	// switchVariationCombo

	switchVariationCombo.setSize(150, buttonH);
	addAndMakeVisible(switchVariationCombo);
	switchVariationCombo.addItem("Switch from start", 1);
	switchVariationCombo.addItem("Switch within beat", 2);
	switchVariationCombo.addItem("Switch within measure", 3);
	switchVariationCombo.addItem("Switch within pattern", 4);

	switchVariationCombo.onChange = [this]
	{
		parent->setSettings();
	};

	// outputChannel
	addAndMakeVisible(outputChannelEditor);
	outputChannelEditor.setSize(210, buttonH);
	outputChannelEditor.onReturnKey = [this]
	{
		parent->setSettings();
	};
	outputChannelEditor.onFocusLost = [this]
	{
		parent->setSettings();
	};

	// keyrange
	addAndMakeVisible(keyRangeFromEditor);
	keyRangeFromEditor.setSize(210, buttonH);
	keyRangeFromEditor.onReturnKey = [this]
	{
		parent->setSettings();
	};
	keyRangeFromEditor.onFocusLost = [this]
	{
		parent->setSettings();
	};
	
	
	addAndMakeVisible(keyRangeToEditor);
	keyRangeToEditor.setSize(210, buttonH);
	keyRangeToEditor.onReturnKey = [this]
	{
		parent->setSettings();
	};
	keyRangeToEditor.onFocusLost = [this]
	{
		parent->setSettings();
	};
	
} // SettingComponent

//////////////////////////////////////////////////////////////////////////////////////////////

SettingComponent::~SettingComponent()
{
} // ~SettingComponent

//////////////////////////////////////////////////////////////////////////////////////////////

void SettingComponent::paint(Graphics& g)
{
	//// Settings
	int labelOffset = 15;
	int lineWidth = 2;
	auto area = getLocalBounds();

	g.fillAll(TopiaryColour::background);
	g.setColour(TopiaryColour::foreground);
	g.setFont(12.0f);
	g.drawText("Settings", lineWidth, lineWidth, 200, labelOffset, juce::Justification::centredLeft);
	
	auto recBounds = area.removeFromBottom(area.getHeight() - labelOffset);
	g.drawRoundedRectangle((float)recBounds.getX() + lineWidth, (float)recBounds.getY() + lineWidth, (float)recBounds.getWidth() - 2 * lineWidth, (float)recBounds.getHeight() - 2 * lineWidth, (float)lineWidth, (float)lineWidth);

	// turn it into inner rectangle
	auto inRecBounds = Rectangle<int>::Rectangle(recBounds.getX() + 3 * lineWidth, recBounds.getY() + 3 * lineWidth, recBounds.getWidth() - 6 * lineWidth, recBounds.getHeight() - 6 * lineWidth);

	inRecBounds.removeFromLeft(10); // spacer left
	auto lineBounds = inRecBounds.removeFromTop(12); // spacer top

	lineBounds = inRecBounds.removeFromTop(buttonH);
	
	// first line
	auto componentBounds = lineBounds.removeFromLeft(100);
	WFFNButton.setBounds(componentBounds);

	lineBounds.removeFromLeft(10);
	componentBounds = lineBounds.removeFromLeft(100);
	notePassThroughButton.setBounds(componentBounds);

	lineBounds.removeFromLeft(10);
	componentBounds = lineBounds.removeFromLeft(45);
	latch1Button.setBounds(componentBounds);

	lineBounds.removeFromLeft(10);
	componentBounds = lineBounds.removeFromLeft(45);
	latch2Button.setBounds(componentBounds);


	// second line

	inRecBounds.removeFromTop(10); // spacer top
	lineBounds = inRecBounds.removeFromTop(buttonH);

	lineBounds.removeFromLeft(70); // space for Output Ch
	componentBounds = lineBounds.removeFromLeft(30);
	outputChannelEditor.setBounds(componentBounds);

	lineBounds.removeFromLeft(100); // space for keyrange from
	componentBounds = lineBounds.removeFromLeft(30);
	keyRangeFromEditor.setBounds(componentBounds);

	lineBounds.removeFromLeft(50); // space for "to"
	componentBounds = lineBounds.removeFromLeft(40);
	keyRangeToEditor.setBounds(componentBounds);

	g.drawText("Output Ch:               Keyrange from                 to", 20, 70, 400, labelOffset + 3, juce::Justification::centredLeft);

	// third line

	inRecBounds.removeFromTop(10); // spacer top
	lineBounds = inRecBounds.removeFromTop(buttonH);
	lineBounds.removeFromLeft(110); // space for Q Var Start
	componentBounds = lineBounds.removeFromLeft(150);
	quantizeVariationStartCombo	.setBounds(componentBounds);

	g.drawText("Q Var Start:", 20, 100, 200, labelOffset + 3, juce::Justification::centredLeft);

	// 4th line

	inRecBounds.removeFromTop(10); // spacer top
	lineBounds = inRecBounds.removeFromTop(buttonH);
	lineBounds.removeFromLeft(110); // space for Q Var Start
	componentBounds = lineBounds.removeFromLeft(150);
	quantizeRunStopCombo.setBounds(componentBounds);

	g.drawText("Q Var End:",20, 130, 200, labelOffset + 3, juce::Justification::centredLeft);

	// 5th line

	inRecBounds.removeFromTop(10); // spacer top
	lineBounds = inRecBounds.removeFromTop(buttonH);
	lineBounds.removeFromLeft(110); // space for Q Var Start
	componentBounds = lineBounds.removeFromLeft(150);
	switchVariationCombo.setBounds(componentBounds);

	g.drawText("Q Var Switch:", 20, 160, 200, labelOffset + 3, juce::Justification::centredLeft);

	// 6th line

	inRecBounds.removeFromTop(10); // spacer top
	lineBounds = inRecBounds.removeFromTop(buttonH);
	lineBounds.removeFromLeft(110); // space for Q Var Start
	componentBounds = lineBounds.removeFromLeft(150);
	noteOrderCombo.setBounds(componentBounds);

	g.drawText("Follow Note:", 20, 190, 200, labelOffset + 3, juce::Justification::centredLeft);

	// 7th line

	inRecBounds.removeFromTop(10); // spacer top
	lineBounds = inRecBounds.removeFromTop(buttonH);
	lineBounds.removeFromLeft(110); // space for Q Var Start
	componentBounds = lineBounds.removeFromLeft(200);
	nameEditor.setBounds(componentBounds);

	g.drawText("Preset name:", 20, 220, 200, labelOffset + 3, juce::Justification::centredLeft);

	// 8th line

	inRecBounds.removeFromTop(10); // spacer top
	lineBounds = inRecBounds.removeFromTop(buttonH);
	componentBounds = lineBounds.removeFromLeft(150);
	saveButton.setBounds(componentBounds);
	lineBounds.removeFromLeft(20);
	componentBounds = lineBounds.removeFromLeft(150);
	loadButton.setBounds(componentBounds);
	
} // paint

//////////////////////////////////////////////////////////////////////////////////////////////

void SettingComponent::resized()
{
} // resized

//////////////////////////////////////////////////////////////////////////////////////////////

void SettingComponent::setParent(TopiaryRiffzMasterComponent* p)
{
	parent = p;
	
} // setParent

//////////////////////////////////////////////////////////////////////////////////////////////