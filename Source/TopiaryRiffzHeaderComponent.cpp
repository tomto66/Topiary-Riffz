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
#include "TopiaryRiffzComponent.h"

TopiaryRiffzHeaderComponent::TopiaryRiffzHeaderComponent()
{
	setSize(TopiaryRiffzComponent::width, TopiaryRiffzComponent::headerHeigth);
	addAndMakeVisible(transportComponent);
	addAndMakeVisible(variationButtonsComponent);
	addAndMakeVisible(warningEditor);
	warningEditor.setReadOnly(true);
	warningEditor.setColour(TextEditor::backgroundColourId, TopiaryColour::orange);
	warningEditor.setColour(TextEditor::textColourId, Colours::lightyellow);
	warningEditor.setColour(TextEditor::outlineColourId, TopiaryColour::orange);
	addAndMakeVisible(timeEditor);
	timeEditor.setVisible(false);
	timeEditor.setEnabled(false);
	addAndMakeVisible(lockedEditor);
	lockedEditor.setText("PLUGIN STATE LOCKED");
	lockedEditor.setReadOnly(true);
	lockedEditor.setColour(TextEditor::backgroundColourId, TopiaryColour::orange);
	lockedEditor.setColour(TextEditor::textColourId, Colours::lightyellow);
	lockedEditor.setColour(TextEditor::outlineColourId, TopiaryColour::orange);
	
}

/////////////////////////////////////////////////////////////////////////

TopiaryRiffzHeaderComponent::~TopiaryRiffzHeaderComponent()
{ 	
	riffzModel->removeListener((ActionListener*)this);
}

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzHeaderComponent::setModel(TopiaryRiffzModel* m)
{
	riffzModel = m;
	variationButtonsComponent.setModel(m);
	transportComponent.setModel(m);
	riffzModel->setListener((ActionListener*)this);
	variationButtonsComponent.checkModel();
	transportComponent.checkModel();
	warningEditor.setVisible(false);
	
	if (m->getLockState())
		lockedEditor.setVisible(true);
	else
		lockedEditor.setVisible(false);
} // setModel

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzHeaderComponent::resized()
{ // bounds are handled by viewer
}

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzHeaderComponent::paint(Graphics& g) {
	// load background
	g.fillAll(TopiaryColour::background );
	Image background = ImageCache::getFromMemory(BinaryData::topiaryRiffzLogo_75_png, BinaryData::topiaryRiffzLogo_75_pngSize);
	g.drawImageAt(background, 0, 0);
	auto area = getLocalBounds();
	
	variationButtonsComponent.setBounds(640,30 ,350,45);
	transportComponent.setBounds(295, 30, 350,45);

	warningEditor.setBounds(645, 5, 350, 18);
	lockedEditor.setBounds(295, 5, 150, 18);
	timeEditor.setBounds(402, 17, 70, 18);

}

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzHeaderComponent::actionListenerCallback(const String &message)
{
	
	if (message.compare(MsgLockState) == 0)
	{
		if (riffzModel->getLockState())
			lockedEditor.setVisible(true);
		else
			lockedEditor.setVisible(false);
	}
	if (message.compare(MsgWarning) == 0)
	{
		warningEditor.setText(riffzModel->getLastWarning());
		warningEditor.setVisible(true);
		startTimer(3000);
	}
	else if (message.compare(MsgTiming) == 0) 
	{
		timeEditor.setVisible(true);
		getTime();
		startTimer(5000);
	}
	else
	{
		if (message.compare(MsgVariationSelected) == 0)
			variationButtonsComponent.checkModel();
		else if (message.compare(MsgTransport) == 0)
			transportComponent.checkModel();
		else if (message.compare(MsgVariationEnables) == 0)
			variationButtonsComponent.getEnabled();
		
	}
}  // actionListenerCallBack

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzHeaderComponent::timerCallback()
{
	if (warningEditor.isVisible())
	{
		warningEditor.setVisible(false);
	}
	else if (timeEditor.isVisible())
	{
		timeEditor.setVisible(false);
	}
	stopTimer();
}



