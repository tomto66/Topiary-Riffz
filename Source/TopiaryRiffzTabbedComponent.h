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
#include "TopiaryRiffzMasterComponent.h"
#include "TopiaryRiffzLogComponent.h"
#include "TopiaryRiffzVariationComponent.h"
#include "TopiaryRiffzComponent.h"
#include "TopiaryRiffzUtilityComponent.h"
#include "TopiaryRiffzPatternComponent.h"

class TopiaryRiffzTabbedComponent:  public Component, ActionListener
{
public:
	TopiaryRiffzTabbedComponent();
    ~TopiaryRiffzTabbedComponent();
	void setModel(TopiaryRiffzModel* model);
    void paint (Graphics&) override;
    void resized() override;  
	void actionListenerCallback(const String &message) override;

private:
	TopiaryRiffzModel* riffzModel;
	TabbedComponent beatsTabs;
	TopiaryRiffzMasterComponent masterComponent;
	TopiaryRiffzVariationComponent variationComponent;
	TopiaryRiffzLogComponent logComponent;
	TopiaryRiffzUtilityComponent utilityComponent;
	TopiaryRiffzPatternComponent patternComponent;

	int tabHeight;
	int height;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopiaryRiffzTabbedComponent)
};
