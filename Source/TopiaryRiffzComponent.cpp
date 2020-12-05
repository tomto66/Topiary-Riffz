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

const int TopiaryRiffzComponent::headerHeigth = 75;
const int TopiaryRiffzComponent::width = 1000;
const int TopiaryRiffzComponent::heigth = 500;

TopiaryRiffzComponent::TopiaryRiffzComponent()
{   
}

void TopiaryRiffzComponent::setModel(TopiaryRiffzModel* model)
{
	riffzModel = model;
	topiaryRiffzHeaderComponent.setModel(riffzModel); // size set in headerComponent
	addAndMakeVisible(topiaryRiffzHeaderComponent);
	topiaryRiffzTabbedComponent.setModel(riffzModel);
	addAndMakeVisible(topiaryRiffzTabbedComponent); // size set in tabbedComponent 	
}

/////////////////////////////////////////////////////////////////////////

TopiaryRiffzComponent::~TopiaryRiffzComponent()
{	
}



/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzComponent::resized()
{ 
}

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzComponent::paint(Graphics& g) {
	auto area = getLocalBounds();
	
	g.fillAll(TopiaryColour::background);
	topiaryRiffzTabbedComponent.setBounds(area.removeFromBottom(TopiaryRiffzComponent::heigth-TopiaryRiffzComponent::headerHeigth-10));
	topiaryRiffzHeaderComponent.setBounds(area.removeFromTop(TopiaryRiffzComponent::headerHeigth));
	
}









