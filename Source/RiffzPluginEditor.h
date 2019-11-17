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
#include "RiffzPluginProcessor.h"
#include "TopiaryRiffzComponent.h"

#ifndef TOPIARYMODEL
#define TOPIARYMODEL TopiaryRiffzModel
#endif

#define TOPIARYEDITORCOMPONENT TopiaryRiffzComponent
#include "../../Topiary/Source/PluginEditor.h"

/*

//==============================================================================


class TopiaryRiffzAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    TopiaryRiffzAudioProcessorEditor (TopiaryRiffzAudioProcessor&);
    ~TopiaryRiffzAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TopiaryRiffzAudioProcessor& processor;
	TopiaryRiffzModel* riffzModel;

	TopiaryRiffzComponent topiaryRiffzComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopiaryRiffzAudioProcessorEditor)
};
*/