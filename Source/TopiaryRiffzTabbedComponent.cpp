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

#include "TopiaryRiffzComponent.h"
#include "TopiaryRiffzTabbedComponent.h"

TopiaryRiffzTabbedComponent::TopiaryRiffzTabbedComponent() : beatsTabs(TabbedButtonBar::Orientation::TabsAtTop)   // above needed otherwise the header declares TabbedButtonBar beatsTabs (without orientation!!!)
	                                                         // see C++ member initialisation lists for explanation !
															
{
	tabHeight = TopiaryRiffzComponent::heigth - TopiaryRiffzComponent::headerHeigth - 30;
	height = TopiaryRiffzComponent::heigth - TopiaryRiffzComponent::headerHeigth;

	addAndMakeVisible(beatsTabs);
	// do NOT EVER make the tab components themselves visible - it'll screw up royally!!!!

	setSize(TopiaryRiffzComponent::width, height);

	masterComponent.setSize(TopiaryRiffzComponent::width, tabHeight);
	beatsTabs.addTab("Master", TopiaryColour::background, &masterComponent, true);

	patternComponent.setSize(TopiaryRiffzComponent::width, tabHeight);
	beatsTabs.addTab("Patterns", TopiaryColour::background, &patternComponent, true);

	variationComponent.setSize(TopiaryRiffzComponent::width, tabHeight);
	beatsTabs.addTab("Variations", TopiaryColour::background, &variationComponent, true);
	
	utilityComponent.setSize(TopiaryRiffzComponent::width, tabHeight);
	beatsTabs.addTab("Utility", TopiaryColour::background, &utilityComponent, true);

	logComponent.setSize(TopiaryRiffzComponent::width, tabHeight);
	beatsTabs.addTab("Log", TopiaryColour::background, &logComponent, true);
	
	setSize(TopiaryRiffzComponent::width, TopiaryRiffzComponent::heigth - TopiaryRiffzComponent::headerHeigth);

}

/////////////////////////////////////////////////////////////////////////

TopiaryRiffzTabbedComponent::~TopiaryRiffzTabbedComponent()
{
	riffzModel->removeListener((ActionListener*)this);
}

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzTabbedComponent::setModel(TopiaryRiffzModel* model) 
{
	riffzModel = model;
	masterComponent.setModel(riffzModel);
	logComponent.setModel(riffzModel);
	variationComponent.setModel(riffzModel);
	utilityComponent.setModel(riffzModel);
	patternComponent.setModel(riffzModel);
	riffzModel->setListener((ActionListener*)this);
}

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzTabbedComponent::paint(Graphics& g)
{
	UNUSED(g)
	auto area = getLocalBounds();
	beatsTabs.setBounds(area);
}

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzTabbedComponent::resized()
{
}

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzTabbedComponent::actionListenerCallback(const String &message)
{
	if (message.compare(MsgTransport) == 0)
	{
		int b, n, d, runState;
		bool o, w;

		riffzModel->getTransportState(b, n, d, runState, o, w);
		if ((runState == Topiary::Running) || (runState == Topiary::Armed) || (runState == Topiary::Ending))
		{
			masterComponent.setEnabled(false);
			variationComponent.setEnabled(false);
			utilityComponent.setEnabled(false);
			patternComponent.setEnabled(false);
		}
		else
		{
			masterComponent.setEnabled(true);
			variationComponent.setEnabled(true);
			utilityComponent.setEnabled(true);
			patternComponent.setEnabled(true);
		}
	}
} // actionListenerCallback

/////////////////////////////////////////////////////////////////////////