/////////////////////////////////////////////////////////////////////////////
/*
This file is part of Topiary Riffz, Copyright Tom Tollenaere 2018-21.

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
#include "../JuceLibraryCode/JuceHeader.h"
#include "TopiaryRiffzModel.h"
#include "TopiaryRiffzTransportComponent.h"
#include "TopiaryRiffzVariationButtonsComponent.h"

class TopiaryRiffzHeaderComponent : public Component,  ActionListener, Timer
{
public:

	TopiaryRiffzHeaderComponent();
	~TopiaryRiffzHeaderComponent();
	void setModel(TopiaryRiffzModel* m);
	void paint(Graphics& g) override;
	void resized() override;
	void actionListenerCallback(const String &message) override;
	void timerCallback();

private:
	TopiaryRiffzTransportComponent transportComponent;
	TopiaryRiffzVariationButtonsComponent variationButtonsComponent;
	TopiaryRiffzModel* riffzModel;
	TextEditor warningEditor;
	TextEditor lockedEditor;
	TextEditor timeEditor;

	//////////////////////////////////////////////////////

	void getTime()
	{
		int m, b;
		String timing;
		
		riffzModel->getTime(m, b);
		timing = String(m) + " : " + String(b);

		if (timeEditor.getText().compare(timing) !=0)
			timeEditor.setText(timing);

	} // getTime

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopiaryRiffzHeaderComponent)
};



