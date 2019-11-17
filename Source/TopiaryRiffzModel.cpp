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

#include "TopiaryRiffzModel.h"

// following has std model code that can be included (cannot be in TopiaryModel because of variable definitions)
#include"../../Topiary/Source/TopiaryModel.cpp.h"

void TopiaryRiffzModel::saveStateToMemoryBlock(MemoryBlock& destData)
{
	addParametersToModel();  // this adds an XML element "Parameters" to the model
	AudioProcessor::copyXmlToBinary(*model, destData);
}

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::restoreStateFromMemoryBlock(const void* data, int sizeInBytes)
{
	model.reset(AudioProcessor::getXmlFromBinary(data, sizeInBytes));
	restoreParametersToModel();
}

/////////////////////////////////////////////////////////////////////////

TopiaryRiffzModel::TopiaryRiffzModel()
{

	Log(String("Topiary Riffz V ") + String(xstr(JucePlugin_Version)) + String(" (c) Tom Tollenaere 2019-2020."), Topiary::LogType::License);
	Log(String(""), Topiary::LogType::License);
	Log(String("Topiary Riffz is free software : you can redistribute it and/or modify"), Topiary::LogType::License);
	Log(String("it under the terms of the GNU General Public License as published by"), Topiary::LogType::License);
	Log(String("the Free Software Foundation, either version 3 of the License, or"), Topiary::LogType::License);
	Log(String("(at your option) any later version."), Topiary::LogType::License);
	Log(String(""), Topiary::LogType::License);
	Log(String("Topiary Riffz is distributed in the hope that it will be useful,"), Topiary::LogType::License);
	Log(String("but WITHOUT ANY WARRANTY; without even the implied warranty of"), Topiary::LogType::License);
	Log(String("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the"), Topiary::LogType::License);
	Log(String("GNU General Public License for more details."), Topiary::LogType::License);
	Log(String(""), Topiary::LogType::License);
	Log(String("VST PlugIn Technology by Steinberg Media Technologies."), Topiary::LogType::License);
	Log(String(""), Topiary::LogType::License);

	// give some of the children yourself as model
	poolList.setRiffzModel(this);
	for (int p = 0; p < 8; p++)
		patternData[p].setRiffzModel(this);

	fixedOutputChannels = true;

	/////////////////////////////////////
	// variations initialisation
	/////////////////////////////////////

	for (int i = 0; i < 8; ++i)
	{
		variation[i].patternToUse = -1;				// index in patterndata
		
		variation[i].lenInMeasures = 0;
		variation[i].pattern.patLenInTicks = 0;
		variation[i].type = Topiary::VariationTypeSteady;								// indicates that once pattern played, we no longer generate notes! (but we keep running (status Ended) till done
		variation[i].ended = false;

		variation[i].name = "Variation " + String(i + 1);
		variation[i].enabled = false;
		variation[i].randomizeNotes = false;
		variation[i].randomizeNotesValue = 100;
		variation[i].swing = false;
		variation[i].swingValue = 0;
		variation[i].randomizeVelocity = false;
		variation[i].velocityValue = 0;
		variation[i].velocityPlus = true;
		variation[i].velocityMin = true;

		variation[i].randomizeTiming = false;
		variation[i].timingValue = 0;
		variation[i].timingPlus = true;
		variation[i].timingMin = true;

		variation[i].offsetVelocity = false;
		variation[i].swingQ = TopiaryRiffzModel::SwingQButtonIds::SwingQ4;
		for (int j = 0; j < 4; j++)
		{
			variation[i].enablePool[j] = true;
			variation[i].poolChannel[j] = 10;
			variation[i].randomizeNotePool[j] = false;
			variation[i].swingPool[j] = false;
			variation[i].velocityPool[j] = false;
			variation[i].timingPool[j] = false;
			variation[i].velocityOffset[j] = 0;
		}
	}

	/////////////////////////////////////
	// PatternData initialization
	/////////////////////////////////////


	for (int i = 0; i < 8; ++i)
	{
		patternData[i].numItems = 0;
	}
	/////////////////////////////////////
	// VariationSwitch initialization
	/////////////////////////////////////

	int ccStart = 22;

	for (int i = 0; i < 8; ++i)
	{
		variationSwitch[i] = ccStart;
		ccStart++;
	}

	ccVariationSwitching = true;
	variationSwitchChannel = 0;

	overrideHostTransport = true;

} // TopiaryRiffzModel

//////////////////////////////////////////////////////////////////////////////////////////////////////

TopiaryRiffzModel::~TopiaryRiffzModel()
{

} //~TopiaryRiffzModel

///////////////////////////////////////////////////////
// Pattern methods
///////////////////////////////////////////////////////

int TopiaryRiffzModel::getPatternLengthInMeasures(int i)
{
	// length of pattern in Measures

	jassert( (i<patternList.getNumItems()) && (i>=-1));

	return patternList.dataList[i].measures;

} // getPatternLengthInMeasures

//////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setPatternLengthInMeasures(int i, int l)
{
	// l: length of pattern in Measures
	// i: pattern index
	// called by setPatternLength
	jassert((i < patternList.getNumItems()) && (i >= 0));

	patternList.dataList[i].measures = l;

} // setPatternLengthInMeasures

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::deletePattern(int deletePattern)
{
	jassert(deletePattern > -1); // has to be a valid row to delete
	jassert(deletePattern < patternList.getNumItems()); // number has to be smaller than number of children (it starts at 0)
	
	patternList.del(deletePattern);
	patternData[deletePattern].numItems = 0; // make sure it's classified as empty

	// now shift down any other patterns in patternData
	for (int p = deletePattern; p < (patternList.numItems); p++)
	{
		TopiaryPattern dummy;
		dummy = patternData[p];
		patternData[p] = patternData[p + 1];
		patternData[p + 1] = dummy;
	}

	Log("Pattern "+String(deletePattern)+" deleted.", Topiary::LogType::Info);
	
	// if there are variations that use this pattern, they need to be re-initialized (send in -1 in the initialize code)
	for (int j = 0; j < 8; j++)
	{
		if (variation[j].patternToUse == deletePattern)
			initializePatternToVariation(-1, j);
	}
			
	broadcaster.sendActionMessage(MsgVariationEnables); // may need to disable the variation!

	// if there are variations that use patterns higher than this one, lower their patternToUse by -1!
	
	for (int j = 0; j < 8; j++)
	{
		if (variation[j].patternToUse > (deletePattern))
		{
			variation[j].patternToUse--;
		}
	}

	broadcaster.sendActionMessage(MsgVariationDefinition);  // something may have changed to the currently shown variation (it might be disabled)
	

} // deletePattern

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::addPattern()
{
	if (patternList.getNumItems() >= patternList.maxItems) // num
	{
		Log("Number of patterns is limited to 8.", Topiary::LogType::Warning);
		return;
	}

	patternList.add();
	patternData[patternList.getNumItems() - 1].patLenInTicks = numerator * Topiary::TicksPerQuarter;
	patternData[patternList.getNumItems() - 1].numItems = 0;

	Log("New empty pattern created.", Topiary::LogType::Info);
	

	if (getNumPatterns() == 0)
		initializeVariations();

} // addPattern

///////////////////////////////////////////////////////////////////////

bool TopiaryRiffzModel::insertPatternFromFile(int patternIndex, bool overload)
{   // patternIndex starts at 0
	// overload = true means do not delete pattern contents
	jassert(patternIndex > -1);  // nothing selected in the model

	auto directory = File::getSpecialLocation(File::userHomeDirectory);

	if (filePath.compare("") == 0)
		directory = File::File(filePath);

	FileChooser myChooser("Please select MIDI file to load...", filePath, "*.mid");

	bool success = false;

	if (myChooser.browseForFileToOpen())
	{
		auto f = myChooser.getResult();

		filePath = f.getParentDirectory().getFullPathName();

		jassert(patternIndex > -1);  // nothing selected in the model
		jassert(patternIndex < getNumPatterns());

		// if there is data in the parent, delete it unless we are overloading
		if ((patternData[patternIndex].numItems != 0) && !overload) patternData[patternIndex].numItems = 0;

		int patternMeasures = 0;
		int lenInTicks = 0;
		success = loadMidiDrumPattern(f, patternIndex, patternMeasures, lenInTicks);

		if (success)
		{
			// all OK now, so we set the length and the name now
			// CAREFUL - len in measures & name are in patternList, patlenInTicks is in the pattern[] structure!! (because that one is not edited in a table !!!!
			patternList.dataList[patternIndex].name = f.getFileName();

			if ((lenInTicks != patternData[patternIndex].patLenInTicks) && overload)
			{
				if (lenInTicks > patternData[patternIndex].patLenInTicks)
				{
					patternList.dataList[patternIndex].measures = patternMeasures;
					patternData[patternIndex].patLenInTicks = lenInTicks;
					Log("New pattern longer than existing one.", Topiary::LogType::Warning);
					Log("Pattern has been stretched but second part does not contain original note data.", Topiary::LogType::Warning);
				}
				else
				{
					Log("New pattern shorter than existing one.", Topiary::LogType::Warning);
					Log("Second pattern part does not contain overloaded note data.", Topiary::LogType::Warning);
				}
			}
			else
			{
				patternList.dataList[patternIndex].measures = patternMeasures;
				patternData[patternIndex].patLenInTicks = lenInTicks;
			}
			rebuildPool(true);

			bool deassigned = false;

			// if any variations use this pattern, unassign them 
			for (int v = 0; v < 8; v++)
			{
				if (variation[v].patternToUse == patternIndex)
				{
					initializePatternToVariation(-1, v);
					Log("Variation " + String(v + 1) + " deassigned from this pattern.", Topiary::LogType::Warning);
					broadcaster.sendActionMessage(MsgWarning);  // warn the user in the header
					deassigned = true;
				}
			}

			if (deassigned)
			{
				broadcaster.sendActionMessage(MsgVariationDefinition);
			}
			
		}
		
	}
	else
		success = false;

	return success;

} // insertPatternFromFile

//////////////////////////////////////////////////////////////////////////////////
// NOTE POOL METHODS
//////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::deletePoolNote(int i)
{
	// deletes a note from the note list

	jassert(i > -1); // has to be a valid row to delete
	jassert(i < poolList.getNumItems()); // number has to be smaller than number of children (it starts at 0) 

	//remove from the poollist - find it first
	poolList.del(i);

	broadcaster.sendActionMessage(MsgPattern);
	Log("Note deleted.", Topiary::LogType::Info);
	
} // deletePoolNote

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::addPoolNote()
{
	// adds a note to the pool list

	poolList.add();
	poolList.sortByNote();
	//rebuildPool(false);
	broadcaster.sendActionMessage(MsgNotePool);
	Log("Note created.", Topiary::LogType::Info);

} //addPoolNote

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::rebuildPool(bool clean)
{	// looks at all patterns and rebuilds the pool
	// existing pool data is not overwritten!
	// called after inserting midi file, recording and pattern edits
	// deleting a pattern does NOT rebuild the pool!
	// if (clean) then notes not in patterns are deleted
	// generate a warning if there are duplicate notes in the pool

	SortedSet<int> set;
	int numItems = poolList.getNumItems();

	// loop over all patterns and make set of unique ones

	for (int p = 0; p < getNumPatterns(); p++)
	{
		if (patternData[p].numItems != 0)
			for (int i=0;i< patternData[p].numItems; i++)
			{
				set.add(patternData[p].dataList[i].note);
			}
	}

	// loop over the pool and add "**" to descriptions (indicating note not used)
	for (int p=0; p< numItems; p++)
	{

		if (poolList.dataList[p].description.substring(0, 2).compare("**") != 0)
			poolList.dataList[p].description = String("**") +poolList.dataList[p].description;
		if (clean)
			poolList.dataList[p].ID = -99; // so we can maybe delete it later
	}

	
	// loop over the set, see if in the pool
	// if so do housekeeping (note names and GM drum map names), if needed remove "**" from label 
	// if not, add to pool
	for (auto it = set.begin(); it != set.end(); ++it)
	{
		int index = poolList.isNoteInPool(*it);
		if (index != -1)
		{
			// note was already in the pool
			// cleanup; ensure label is correct
			poolList.dataList[index].label = MidiMessage::getMidiNoteName(*it, true, true, 5);

			if (poolList.dataList[index].description.substring(0, 2).compare("**") == 0) // if note in the pool, delete the ** and reset to poolnote
			{
				poolList.dataList[index].description = poolList.dataList[index].description.substring(2);
				poolList.dataList[index].ID = 0; // as long as it's not -99 - it will get renumbered at the end
			}
			
		}
		else
		{
			// this note (it) was not in the pool yet - add it
			poolList.add();
			poolList.dataList[numItems].note = *it;
			poolList.dataList[numItems].pool = 1;
			poolList.dataList[numItems].label = MidiMessage::getMidiNoteName(*it, true, true, 5);
			poolList.dataList[numItems].description = "";
			poolList.dataList[numItems].ID = 0; // just make sure it is not -99
			numItems++;
		}
	}

	if (clean)
		// delete all IDs of -99
		for (int p=0; p< numItems; p++)
			if (poolList.dataList[p].ID == -99)
			{
				poolList.del(p);
				numItems--; // because otherwise we jump over one!	
			}

	poolList.renumber(); 

	broadcaster.sendActionMessage(MsgNotePool);

	// warn if there are duplicate notes in the set

	set.clear();

	for (int p = 0; p < numItems; p++)
	{
		if (set.contains(poolList.dataList[p].note))
			Log("Note " + poolList.dataList[p].label + " is duplicate in the note pool.", Topiary::LogType::Warning);
		else
			set.add(poolList.dataList[p].note);
	}

	poolList.sortByNote();
} // rebuildPool

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setGMDrumMapLabels()
{
	for (int i = 0; i < poolList.getNumItems(); i++) 
	{
		int note = poolList.dataList[i].note;
		
		// remember if there was a ** in the description and keep that if it was there
		String twoStars = poolList.dataList[i].description.substring(0, 2);
		if (!twoStars.equalsIgnoreCase("**"))
			twoStars = "";
		poolList.dataList[i].description = twoStars + String( MidiMessage::getRhythmInstrumentName(note));
	}
} // setGMDrumMapLabels

///////////////////////////////////////////////////////////////////////

bool TopiaryRiffzModel::getFixedOutputChannels()
{
	return fixedOutputChannels;
	
} // getFixedOutputChannels

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setFixedOutputChannels(bool b)
{
	fixedOutputChannels = b;

} // setFixedOutputChannels

///////////////////////////////////////////////////////////////////////
//  VARIATIONS
///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::initializeVariationsForRunning()
{
	// careful; this code is different in various plugins - don't think it's generic!!!
	for (int v = 0; v < 8; v++)
	{
		variation[v].ended = false;
		variation[v].currentPatternChild = 0;
	}
} // initializeVariationsForRunning

///////////////////////////////////////////////////////////////////////
// Variation Parameters
///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getVariationDefinition(int i, bool& enabled, String& vname, int& patternId, bool enables[4], int poolChannel[4], int& type)
{
	jassert(i < 8); // goes from 0-7
	vname = variation[i].name;
	enabled = variation[i].enabled;
	type = variation[i].type;
	patternId = variation[i].patternToUse;
	for (int j = 0; j < 4; j++)
	{
		enables[j] = variation[i].enablePool[j];
		poolChannel[j] = variation[i].poolChannel[j];
	}
	// need to look up the ID in the items in the patternCombo, and set the patternCombo selection

} // getVariationDefinition

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setVariationDefinition(int i, bool enabled, String vname, int patternId, bool enables[4],  int poolChannel[4], int type)
{
	// write to model
	// assume the data has been validated first!
	// make sure that overrideHost is set to TRUE if there are no enabled variations!!!

	jassert(i < 8); // goes from 0-7
	if (getNumPatterns() == 0) return;  // should not really happen unless @ initialization - but in that case we shouldn't write to the model

	// if patternId == -1 and that means we are not touching a not-initialized variation - do nothing
	if (patternId == -1) return;

	//int lenInMeasures = getPatternLengthInMeasures(patternId);

	variation[i].name = vname;
	variation[i].enabled = enabled;
	variation[i].type = type;
	if (!enabled)
	{
		// make sure to enable hostOverride if there are no enabled variations
		// otherwise host might try to run the plugin when these is nothing to run - it might hang
		bool ok = false;
		for (int v = 0; v < 8; v++)
		{
			if (variation[v].enabled)
				ok = true;
		}

		if (!ok && !overrideHostTransport)
		{
			setOverrideHostTransport(true);
			Log("Host overridden because no variation enabled; don't forget to re-enablde if needed.", Topiary::Warning);
		}
	}

	// if we are changing patterns there is no need to pass in from/to - those will be reinitialized
	if (patternId == variation[i].patternToUse)
	{
		for (int j = 0; j < 4; j++)
		{
			variation[i].enablePool[j] = enables[j];
			variation[i].pattern.patLenInTicks = patternData[variation[i].patternToUse].patLenInTicks;
		}
	}
	else
		initializePatternToVariation(patternId, i);

	if (fixedOutputChannels)
	{
		for (i = 0; i<8; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				variation[i].poolChannel[j] = poolChannel[j];
			}
			// regenerate!
			generateVariation(i, -1);
		}

	}
	else
	{
		for (int j = 0; j < 4; j++)
		{
			variation[i].poolChannel[j] = poolChannel[j];
		}
		// regenerate!
		generateVariation(i, -1);
	}

	broadcaster.sendActionMessage(MsgVariationDefinition);
	broadcaster.sendActionMessage(MsgVariationEnables);  // may need to update the enable buttons

}  // setVariationDefinition

//////////////////////////////////////////////////////////////////////

bool TopiaryRiffzModel::validateVariationDefinition(int i, bool enable, String vname, int patternId)
{
	// return false if parameters do not make sense
	// refuse to enable a variation if there are no patterns
	// refuse to enable a variation if the pool settings make no sense and do not match e.g. length of pattern
	// if patternId == -1 check if it was -1 in the model; if so give is a pass; otherwise complain that pattern needs to be selected

	jassert(i < 8); // goes from 0-7

	if (getNumPatterns() == 0) return false;  // in this case the variations tab should be disabled anyway

	bool refuse = false;

	if (enable && (patternId == -1))
	{
		Log("Variation " + String(i + 1) + ": Pattern needs to be selected.", Topiary::LogType::Warning);
		refuse = true;
		return refuse;
	}

	if ((patternId < 0) && (variation[i].patternToUse == -1)) return false;  // don't do further checks - pattern was uninitialized to start with

	if ((patternId < 0) && (variation[i].patternToUse != -1))
	{
		Log("Variation "+String(i+1)+": Pattern needs to be selected.", Topiary::LogType::Warning);
		refuse = true;
		return refuse;
	}

	if (!name.compare("")) {
		Log("Variation needs a name.", Topiary::LogType::Warning);
		refuse = true;
	}

	// when changing the pattern; do not validate the from-to : those will be reinitializen later

	if (patternId != variation[i].patternToUse)
	{
		return false;
	}

	int lenInTicks = patternData[patternId].patLenInTicks;
	if (lenInTicks < 1)
	{
		Log("Pattern must have length > 0 in order to be used here.", Topiary::LogType::Warning);
		return true;
	}

	int lenInMeasures = getPatternLengthInMeasures(patternId);

	if (lenInMeasures < 1)
	{
		Log("Pattern needs at least 1 measure in order to be used here.", Topiary::LogType::Warning);
		return true;
	}
		
	if (refuse) Log("Current values will not be remembered!", Topiary::LogType::Warning);

	return refuse;

} // validateVariationDefinition

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setRandomizeNotes(int v, bool enable, bool enablePool[4], int value)
{
	variation[v].randomizeNotes = enable;
	variation[v].randomizeNotesValue = value;
	for (int i = 0; i < 4; i++)
	{
		variation[v].randomizeNotePool[i] = enablePool[i];
	}
	generateVariation(v, -1);
	
} // getRandomNotes

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getRandomizeNotes(int v, bool &enable, bool enablePool[4], int &value)
{
	enable = variation[v].randomizeNotes;
	value = variation[v].randomizeNotesValue;
	for (int i = 0; i < 4; i++)
	{
		enablePool[i] = variation[v].randomizeNotePool[i];
	}

} // setRandomNotes

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setSwing(int v, bool enable, bool enablePool[4], int value)
{
	variation[v].swing = enable;
	variation[v].swingValue = value;
	for (int i = 0; i < 4; i++)
	{
		variation[v].swingPool[i] = enablePool[i];
	}

	generateVariation(v, -1);

} // getSwing

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getSwing(int v, bool &enable, bool enablePool[4], int &value)
{
	enable = variation[v].swing;
	value = variation[v].swingValue;
	for (int i = 0; i < 4; i++)
	{
		enablePool[i] = variation[v].swingPool[i];
	}

} // getSwing

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setRandomizeVelocity(int v, bool enable, bool enablePool[4], int value, bool plus, bool min)
{
	variation[v].randomizeVelocity = enable;
	variation[v].velocityValue = value;
	variation[v].velocityPlus = plus;
	variation[v].velocityMin = min;
	for (int i = 0; i < 4; i++)
	{
		variation[v].velocityPool[i] = enablePool[i];
	}

	generateVariation(v, -1);

} // getRandomizeVelocity

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getRandomizeVelocity(int v, bool &enable, bool enablePool[4], int &value, bool &plus, bool &min)
{
	enable = variation[v].randomizeVelocity;
	value = variation[v].velocityValue;
	plus = variation[v].velocityPlus;
	min = variation[v].velocityMin;
	for (int i = 0; i < 4; i++)
	{
		enablePool[i] = variation[v].velocityPool[i];
	}

} // getRandomizeVelocity

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setRandomizeTiming(int v, bool enable, bool enablePool[4], int value, bool plus, bool min)
{
	variation[v].randomizeTiming = enable;
	variation[v].timingValue = value;
	variation[v].timingPlus = plus;
	variation[v].timingMin = min;
	for (int i = 0; i < 4; i++)
	{
		variation[v].timingPool[i] = enablePool[i];
	}

	generateVariation(v, -1);

} // setRandomizeTiming

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getRandomizeTiming(int v, bool &enable, bool enablePool[4], int &value, bool &plus, bool &min)
{
	enable = variation[v].randomizeTiming;
	value = variation[v].timingValue;
	plus = variation[v].timingPlus;
	min = variation[v].timingMin;

	for (int i = 0; i < 4; i++)
	{
		enablePool[i] = variation[v].timingPool[i];
	}

} // getRandomizeTiming

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setSwingQ(int v, int q)
{
	variation[v].swingQ = q;
	generateVariation(v, -1);
}

///////////////////////////////////////////////////////////////////////

int TopiaryRiffzModel::getSwingQ(int v)
{
	return variation[v].swingQ;
}

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getPatterns(String pats[8])
{
	for (int i=0; i < 8; i++)
		pats[i] = "";

	for (int i=0; i< patternList.getNumItems(); i++)
	{
		pats[i] = String(i + 1) + " " + patternList.dataList[i].name;
	}
	
} // getPatterns

///////////////////////////////////////////////////////////////////////

TopiaryPatternList* TopiaryRiffzModel::getPatternList()
{
	return &patternList;
}

///////////////////////////////////////////////////////////////////////

TopiaryPoolList* TopiaryRiffzModel::getPoolList()
{
	return &poolList;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::generateVariation(int i, int measureToGenerate)
{
	// (re)generates variation[i].pattern 

	jassert((i < 8) && (i >= 0));

	if (!variation[i].enabled)
		return;

	if (measureToGenerate == -1)
	{
		// meaning we regererate the lot
		// reset numItems
		variation[i].pattern.numItems = 0;
	}

	int poolNote[128];  // will contain the pool number for the note
	
	int note;
	int pool;

	// poolNote is helper datastructure; given a note, it returns the pool the note is in (careful for -1s; pool numbers in variation are 0-3 but in screen are 1-4

	for (int j = 0; j < 128; j++) poolNote[j] = -1;

	for (int x=0; x<poolList.getNumItems(); x++)

	{
		// loop over the pool notes and put them in the appropriate set
		note = poolList.dataList[x].note;
		pool = poolList.dataList[x].pool; 
		poolNote[note] = pool - 1;
		
	}

	// set every event in the variation to miditype::NOP
	// (because if certain pools are off the notes should not be generated

	TopiaryVariation* var = &(variation[i].pattern);
	TopiaryPattern* pat = &(patternData[variation[i].patternToUse]);
	if (measureToGenerate == -1)
	{
		// make sure it's initialized properly - only done from editor or @ load
		var->numItems = pat->numItems;
		for (int j = 0; j < pat->numItems; j++)
		{
			var->dataList[j].ID = j + 1;
			var->dataList[j].timestamp = pat->dataList[j].timestamp;
			int measre = pat->dataList[i].timestamp % (numerator * Topiary::TicksPerQuarter);
			if ((measre == measureToGenerate) || (measureToGenerate == -1))
				var->dataList[j].midiType = Topiary::MidiType::NOP;
		}

	}
	else if (measureToGenerate == patternList.dataList[variation[i].patternToUse].measures) // we ran over end of pattern
		measureToGenerate = 0;

	for (int j = 0; j < pat->numItems; j++)
	{
		//int measre = pat->dataList[i].timestamp % (numerator * Topiary::TicksPerQuarter);
		int measre = (int) pat->dataList[j].timestamp / (numerator * Topiary::TicksPerQuarter);
		if ((measre == measureToGenerate) || (measureToGenerate == -1))
			var->dataList[j].midiType = Topiary::MidiType::NOP;
	}

	////////////////////////////
	// GENERATE THE POOL NOTES
	////////////////////////////

	for (int j = 0; j < 4; j++)
	{
		// only call generatePool if there are notes in the pool; otherwise generatePool will loop forever!!!) 
		generatePool(i, j, poolNote, measureToGenerate);
	}

	//Logger::outputDebugString("SORTED ------------------------");
	variation[i].pattern.sortByTimestamp();
	/*
	Logger::outputDebugString("------------------------");
	for (int j = 0; j < variation[i].pattern.numItems; j++)
	{
		Logger::outputDebugString("<" + String(j) + "> <ID" + String(variation[i].pattern.dataList[j].ID)+"> Note: " + String(variation[i].pattern.dataList[j].note) + " timestamp " + String(variation[i].pattern.dataList[j].timestamp) + " midiType " + String(variation[i].pattern.dataList[j].midiType));
	}
	Logger::outputDebugString("------------------------");
	*/
} // generateVariation

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void  TopiaryRiffzModel::generateAllVariations(int measureToGenerate)
{
	for (int v = 0; v < 8; v++)
	{
		if (variation[v].enabled)
			generateVariation(v, measureToGenerate);
	}
} // generateAllVariations()

////////////////////////////////////////////////////////////////////////////////////
// move to modelincludes when done

void TopiaryRiffzModel::regenerateVariationsForPattern(int p)
{
	// regenerate the variations using this pattern
	for (int v = 0; v < 8; v++)
	{
		if (variation[v].enabled && (variation[v].patternToUse == p))
			generateVariation(v, -1);
	}

} // regenerateVariationsForPattern

////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setPatternLength(int p, int l, bool keepTail)
{
	// set LenInTicks, based on l in measures
	// delete all events possbily after the ticklength
	
	bool warned = false;

	int newLenInTicks = l * Topiary::TicksPerQuarter * denominator;
	int tickDelta = patternData[p].patLenInTicks - newLenInTicks;
	int measureDelta = tickDelta / (Topiary::TicksPerQuarter * denominator);

	if (patternData[p].patLenInTicks != newLenInTicks)
	{
		bool deleted;

		for (int i=0; i<patternData[p].numItems; i++)

		{
			deleted = false;
			if (tickDelta >0) // only delete notes if the pattern gets shorter
				if ( ((newLenInTicks <= patternData[p].dataList[i].timestamp) && !keepTail) ||
					 (keepTail && (patternData[p].dataList[i].timestamp < tickDelta))  )
				{
					// flag to delete the event

					warned = true;
					deleted = true;
				
					if ((patternData[p].dataList[i].timestamp + patternData[p].dataList[i].length) > newLenInTicks)
						patternData[p].dataList[i].length = patternData[p].dataList[i].length - patternData[p].dataList[i].timestamp;
				}

			if (keepTail)
			{
				// correct measure, beat, tick & timestamps
				patternData[p].dataList[i].timestamp = patternData[p].dataList[i].timestamp - tickDelta;
				patternData[p].dataList[i].measure = patternData[p].dataList[i].measure - measureDelta;
			}

			// make sure note length never runs over total patternlength
			if (patternData[p].dataList[i].timestamp + patternData[p].dataList[i].length >= newLenInTicks)
				patternData[p].dataList[i].length =  newLenInTicks - patternData[p].dataList[i].timestamp -1;

			if (deleted)
			{
				patternData[p].del(i);
				i--;
			}
		}

		patternData[p].patLenInTicks = newLenInTicks;
		setPatternLengthInMeasures(p, l);  
		patternData[p].sortByTimestamp();

		// regenerate variations using this pattern
		regenerateVariationsForPattern(p);
		if (warned)
			Log("Pattern was shortened and MIDI events were lost.", Topiary::LogType::Warning);
		broadcaster.sendActionMessage(MsgPattern);
		broadcaster.sendActionMessage(MsgPatternList);
	}
	
} // setPatternLength

////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::deleteNote(int p ,int n)
{
	// deletes note with ID n from pattern p
	patternData[p].del(n);
	broadcaster.sendActionMessage(MsgPattern);

	Log("Note deleted.", Topiary::LogType::Info);
	regenerateVariationsForPattern(p);
	rebuildPool(false);
	
} // deleteNote

////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::addNote(int p, int n, int v, int l, int timestamp) 
{   
	int measuree = (int)(timestamp / (denominator*Topiary::TicksPerQuarter));
	int t = timestamp % (denominator*Topiary::TicksPerQuarter);
	int beatt = (int)(t / Topiary::TicksPerQuarter);
	t = t % Topiary::TicksPerQuarter;
	patternData[p].add(measuree, beatt, t, timestamp, n, l, v);
	patternData[p].sortByTimestamp(); // will create the ID
	broadcaster.sendActionMessage(MsgPattern);
	regenerateVariationsForPattern(p);

}  // addNote

////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getNote(int p, int ID, int& note, int &velocity, int &timestamp, int &length)  // get note with id ID from pattern p
{
	// get note with id ID from pattern p
	note = patternData[p].dataList[ID].note;
	velocity = patternData[p].dataList[ID].velocity;
	length = patternData[p].dataList[ID].length;
	timestamp = patternData[p].dataList[ID].timestamp;

} // getNote

////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::deleteAllNotes(int p, int n)  // delete all notes equal to ID n from pattern
{
	// get note with id ID from pattern p
	int note = patternData[p].dataList[n].note;

	for (int i=0; i<patternData[p].numItems; i++)
	{
		if (patternData[p].dataList[i].note == note) {
			// delete the child
			patternData[p].del(i);
			i--;
		}
	}

} // deleteAllNotes

////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::validateTableEdit(int p, XmlElement* child, String attribute)
{
	// called by TopiaryTable
	// careful - can be called when editing patterns but also when editing note pool entries!!
	// do processing of user edits to notes and make sure all is consistent
	

	int index = child->getIntAttribute("ID")-1;  // ID 1 is in position [0]
	

	if (attribute.compare("Label") == 0) 
	{
		// set the note value correctly
		
		child->setAttribute("Note", validNoteNumber(child->getStringAttribute("Label")));
		child->setAttribute("Label", validateNote(child->getStringAttribute("Label")));
		if (p != -1)
		{
			patternData[p].dataList[index].note = child->getIntAttribute("Note");
			patternData[p].dataList[index].label = child->getStringAttribute("Label");
		}
		else
		{
			poolList.dataList[index].note = child->getIntAttribute("Note");
			poolList.dataList[index].label = child->getStringAttribute("Label");
			poolList.sortByNote();
		}

		

	}
	else if ((attribute.compare("Measure") == 0) || (attribute.compare("Beat") == 0) || (attribute.compare("Tick") == 0))
	{
		// recalculate timestamp

		if (child->getIntAttribute("Beat") >= denominator)
		{
			child->setAttribute("Beat", 0);
			Log("Beat should be between 0 and " + String(denominator) + ".", Topiary::LogType::Warning);
		}


		int timestamp = child->getIntAttribute("Tick") +
			child->getIntAttribute("Beat")* Topiary::TicksPerQuarter +
			child->getIntAttribute("Measure") * Topiary::TicksPerQuarter * denominator;

		child->setAttribute("Timestamp", String(timestamp));

		patternData[p].dataList[index].timestamp = timestamp;
		patternData[p].dataList[index].measure = child->getIntAttribute("Measure");
		patternData[p].dataList[index].beat = child->getIntAttribute("Beat");
		patternData[p].dataList[index].tick = child->getIntAttribute("Tick");
		patternData[p].sortByTimestamp();
	}

	// what follows detects what we are editing (pattern, notepool or patternlist)

	if (child->hasAttribute("Timestamp"))
	{

		// check note does not run over pattern end
		// make sure note lenght never runs over total patternlength
		if ((child->getIntAttribute("Timestamp") + child->getIntAttribute("Length")) >= patternData[p].patLenInTicks)
		{
			child->setAttribute("Length", String(patternData[p].patLenInTicks - child->getIntAttribute("Timestamp") - 1));
			patternData[p].dataList[index].length = child->getIntAttribute("Length");
			Log("Note shortened so it does not run over pattern end.", Topiary::LogType::Warning);
		}

		patternData[p].sortByTimestamp();

		// regenerate variations that depend on this one
		for (int v = 0; v < 8; v++)
		{
			if (variation[v].patternToUse == p)
				generateVariation(v, -1);
		}
		rebuildPool(false);
		broadcaster.sendActionMessage(MsgPattern);
	}
	else if (child->hasAttribute("Note"))
	{
		// edited the note pool
		broadcaster.sendActionMessage(MsgNotePool);
	}
	else if (child->hasAttribute("Measures"))
	{
		// edited the patternlist
		broadcaster.sendActionMessage(MsgPatternList);
	}
	
} // validateTableEdit

///////////////////////////////////////////////////////////////////////////////////////

int TopiaryRiffzModel::getNumPatterns()
{
	// needed for TopiaryModelIncludes
	return patternList.numItems;
}

///////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getVelocityOffset(int v,  bool& offsetVelocity, int velocityOffset[4])
{
	offsetVelocity = variation[v].offsetVelocity;
	for (int i = 0; i < 4; i++)
		velocityOffset[i] = variation[v].velocityOffset[i];

} // getVelocityOffset

///////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setVelocityOffset(int v, bool offsetVelocity, int velocityOffset[4])
{
	variation[v].offsetVelocity = offsetVelocity;
	for (int i = 0; i < 4; i++)
		variation[v].velocityOffset[i] = velocityOffset[i];

	generateVariation(v, -1);

} // setVelocityOffset

///////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::swapVariation(int from, int to)
{
	jassert((from < 8) && (from >= 0));
	jassert((to < 8) && (to >= 0));
	{
		const GenericScopedLock<CriticalSection> myScopedLock(lockModel);

		intSwap(variation[from].patternToUse, variation[to].patternToUse);
		//intSwap(variation[from].lenInTicks, variation[to].lenInTicks);
		intSwap(variation[from].lenInMeasures, variation[to].lenInMeasures);
		boolSwap(variation[from].ended, variation[to].ended);
		intSwap(variation[from].type, variation[to].type);
		boolSwap(variation[from].enabled, variation[to].enabled);

		stringSwap(variation[from].name, variation[to].name);
		boolSwap(variation[from].randomizeNotes, variation[to].randomizeNotes);
		intSwap(variation[from].randomizeNotesValue, variation[to].randomizeNotesValue);

		boolSwap(variation[from].swing, variation[to].swing);
		intSwap(variation[from].swingValue, variation[to].swingValue);

		boolSwap(variation[from].randomizeVelocity, variation[to].randomizeVelocity);
		intSwap(variation[from].velocityValue, variation[to].velocityValue);
		boolSwap(variation[from].velocityPlus, variation[to].velocityPlus);
		boolSwap(variation[from].velocityMin, variation[to].velocityMin);

		boolSwap(variation[from].randomizeVelocity, variation[to].randomizeVelocity);

		boolSwap(variation[from].randomizeTiming, variation[to].randomizeTiming);
		intSwap(variation[from].timingValue, variation[to].timingValue);
		boolSwap(variation[from].timingPlus, variation[to].timingPlus);
		boolSwap(variation[from].timingMin, variation[to].timingMin);
		boolSwap(variation[from].offsetVelocity, variation[to].offsetVelocity);

		intSwap(variation[from].swingQ, variation[to].swingQ);

		for (int i = 0; i < 4; i++)
		{
			boolSwap(variation[from].enablePool[i], variation[to].enablePool[i]);
			intSwap(variation[from].poolChannel[i], variation[to].poolChannel[i]);
			boolSwap(variation[from].randomizeNotePool[i], variation[to].randomizeNotePool[i]);
			boolSwap(variation[from].swingPool[i], variation[to].swingPool[i]);
			boolSwap(variation[from].velocityPool[i], variation[to].velocityPool[i]);
			boolSwap(variation[from].timingPool[i], variation[to].timingPool[i]);
			intSwap(variation[from].velocityOffset[i], variation[to].velocityOffset[i]);
		}		
	} // end scope of lock

	generateAllVariations(-1);
	
	broadcaster.sendActionMessage(MsgVariationEnables);
	Log("Variation " + String(from + 1) + " swapped with " + String(to + 1) + ".", Topiary::LogType::Info);


} // swapVariation

///////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::copyVariation(int from, int to)
{
	jassert((from < 8) && (from >= 0));
	jassert((to < 8) && (to >= 0));
	{
		const GenericScopedLock<CriticalSection> myScopedLock(lockModel);

		variation[to].patternToUse = variation[from].patternToUse;
		variation[to].lenInMeasures = variation[from].lenInMeasures;
		variation[to].type = variation[from].type;
		variation[to].ended = variation[from].ended;
		variation[to].enabled = variation[from].enabled;
		variation[to].name = variation[from].name + String(" copy ");
		variation[to].randomizeNotes = variation[from].randomizeNotes;
		variation[to].randomizeNotesValue = variation[from].randomizeNotesValue;
		variation[to].swing = variation[from].swing;
		variation[to].swingValue = variation[from].swingValue;
		variation[to].randomizeVelocity = variation[from].randomizeVelocity;
		variation[to].velocityMin = variation[from].velocityMin;
		variation[to].velocityPlus = variation[from].velocityPlus;
		variation[to].randomizeTiming = variation[from].randomizeTiming;
		variation[to].timingValue = variation[from].timingValue;
		variation[to].timingMin = variation[from].timingMin;
		variation[to].timingPlus = variation[from].timingPlus;
		variation[to].offsetVelocity = variation[from].offsetVelocity;
		variation[to].swingQ = variation[from].swingQ;

		for (int i = 0; i < 4; i++)
		{
			variation[to].enablePool[i] = variation[from].enablePool[i];
			variation[to].poolChannel[i] = variation[from].poolChannel[i];
			variation[to].randomizeNotePool[i] = variation[from].randomizeNotePool[i];
			variation[to].swingPool[i] = variation[from].swingPool[i];
			variation[to].velocityPool[i] = variation[from].velocityPool[i];
			variation[to].timingPool[i] = variation[from].timingPool[i];
			variation[to].velocityOffset[i] = variation[from].velocityOffset[i];
		}

		variation[to].currentPatternChild = 0;
		
	} // end oif lock scope

	generateAllVariations(-1);
	
	broadcaster.sendActionMessage(MsgVariationEnables);
	Log("Variation " + String(from + 1) + " copied to " + String(to + 1) + ".", Topiary::LogType::Info);


} // copyVariation

///////////////////////////////////////////////////////////////////////////////////////

bool TopiaryRiffzModel::midiLearn(MidiMessage m)
{
	// called by processor; if midiLearn then learn based on what came in
	const GenericScopedLock<CriticalSection> myScopedLock(lockModel);
	bool remember = learningMidi;
	if (learningMidi)
	{
		bool note = m.isNoteOn();
		bool cc = m.isController();

		if (note || cc)
		{
			// check the Id to learn; tells us what to set
			if ((midiLearnID >= Topiary::LearnMidiId::variationSwitch) && (midiLearnID < (Topiary::LearnMidiId::variationSwitch + 8)))
			{
				// learning variation switch
				if (note)
				{
					ccVariationSwitching = false;
					variationSwitch[midiLearnID] = m.getNoteNumber();
				}
				else
				{
					ccVariationSwitching = true;
					variationSwitch[midiLearnID] = m.getControllerNumber();
				}
				learningMidi = false;
				Log("Midi learned", Topiary::LogType::Warning);
				broadcaster.sendActionMessage(MsgVariationAutomation);	// update utility tab
			}

		}
	}

	return remember;

} // midiLearn

///////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::record(bool b)
{
	// set the recording state (does not record = recording happens in the processor!

	const GenericScopedLock<CriticalSection> myScopedLock(lockModel);

	// check there are pool(s) enabled; otherwise we don't know the channel when recording
	if (b) {
		bool noNotes = true;

		for (int p = 0; p < 4; p++)
		{
			if (variation[variationSelected].enablePool[p])
				noNotes = false;
		}

		if (noNotes)
		{
			Log("Enable at least one note pool in variation.", Topiary::LogType::Warning);
			Log("Can't know which channel to output to.", Topiary::LogType::Warning);
			return;
		}
	}
	else if (recordingMidi)
		processMidiRecording();

	recordingMidi = b;
	// inform transport
	broadcaster.sendActionMessage(MsgTransport);

} // record

///////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::processMidiRecording()
{
	// process recorded events and add to pattern
	
	// loop over the variation and look for IDs == -1
	
	int loopLen = variation[variationSelected].pattern.numItems; 
	int patPos; // where to inser the new events
	TopiaryPattern* pat = &(patternData[variation[variationSelected].patternToUse]);
	TopiaryVariation *var = &(variation[variationSelected].pattern);
	for (int v=0; v < loopLen; v++)
	{
		if (variation[variationSelected].pattern.dataList[v].length == -1)
		{
			if (var->dataList[v].midiType == Topiary::MidiType::NoteOn)
			{
				patPos = pat->numItems;
				pat->add();

				//newChild = new XmlElement("INSERTED");  // the RECDATA elements will get inserted in the pattern when recording done; use tag insertes do we van find the note off for it
				pat->dataList[patPos].ID = -1; // will be renumbered later
				pat->dataList[patPos].note = var->dataList[v].note;
				pat->dataList[patPos].label = MidiMessage::getMidiNoteName(var->dataList[v].note, true, true, 5);
				pat->dataList[patPos].velocity = var->dataList[v].velocity;
				pat->dataList[patPos].timestamp = var->dataList[v].timestamp;
				pat->dataList[patPos].length = 0;
				int measur = (int)floor(var->dataList[v].timestamp / (numerator * Topiary::TicksPerQuarter));
				pat->dataList[patPos].measure = measur;
				int timestamp = var->dataList[v].timestamp - measur * (numerator * Topiary::TicksPerQuarter);
				int bea = (int)floor(timestamp / Topiary::TicksPerQuarter);
				pat->dataList[patPos].beat = bea;
				int tic = timestamp - bea * Topiary::TicksPerQuarter;
				pat->dataList[patPos].tick = tic;
			}
			else if (var->dataList[v].midiType == Topiary::MidiType::NoteOff)
			{
				// find the note and set the length
				int timestamp = var->dataList[v].timestamp; // timestamp of end of note
				int note = var->dataList[v].note; 
				bool cont = true;
				int pIndex = 0;
				int numPats = pat->numItems;

				while (cont && (pIndex<numPats))
				{
					if ((pat->dataList[pIndex].ID == -1) && (pat->dataList[pIndex].note  == note))
					{
						// we found the note to end
						pat->dataList[pIndex].length = timestamp - pat->dataList[pIndex].timestamp;
						pat->dataList[pIndex].ID = -2; // so we don't cover it again
						cont = false;
					}
					pIndex++;
				}
			}
			else jassert(false); // should not happen; should be note on or note off event for now
		}
		
	}
	// resort 
	pat->sortByTimestamp();

	Logger::outputDebugString("------------------------");
	for (int j = 0; j < pat->numItems; j++)
	{
		Logger::outputDebugString("<" + String(j) + "> <ID" + String(pat->dataList[j].ID) + "> Note: " + String(pat->dataList[j].note) + " timestamp " + String(pat->dataList[j].timestamp));
	}
	Logger::outputDebugString("------------------------");

	// rebuild the pool list
	rebuildPool(false);
	generateAllVariations(-1);
	// inform pattern tab
	broadcaster.sendActionMessage(MsgPattern);
	
}  // processMidiRecording

///////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::duplicatePattern(int p)  
{
	jassert(p > -1); // has to be a valid row 
	if (getNumPatterns() > 6)
	{
		Log("Cannot duplicate; number of patterns limited to 8.", Topiary::LogType::Warning);
		return;
	}

	patternList.duplicate(p);
	
	// duplicate the patterndata
	patternData[getNumPatterns() - 1].duplicate(&(patternData[p]));

	broadcaster.sendActionMessage(MsgPatternList);
	Log("Duplicate pattern created.", Topiary::LogType::Info);

} // duplicatePattern

///////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::clearPattern(int p)
{
	jassert(p > -1); // has to be a valid row to delete
	jassert(p < getNumPatterns());

	patternData[p].numItems = 0;
	generateAllVariations(-1);

	Log("Pattern cleared.", Topiary::LogType::Info);

} // clearPattern

///////////////////////////////////////////////////////////////////////////////////////

TopiaryPattern* TopiaryRiffzModel::getPattern(int p)
{
	jassert( (p < getNumPatterns()) || (p==0));
	jassert(p >= 0);
	
	return &(patternData[p]);
}

///////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::quantize(int p, int ticks)
{
	patternData[p].quantize(ticks);
	for (int i=0; i<patternData[p].numItems; i++)
		timestampToMBT(patternData[p].dataList[i].timestamp, patternData[p].dataList[i].measure, patternData[p].dataList[i].beat, patternData[p].dataList[i].tick);
} // quantize

///////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::initializePreviousSteadyVariation()
{
	previousSteadyVariation = 0;
	for (int v = 0; v < 8; v++)
	{
		if ((variation[v].type == Topiary::VariationTypeSteady) || variation[v].enabled)
		{
					previousSteadyVariation = v;
					v = 8;
		}
	}

	// if this is not a steady one (e.g. there are no steady ones), find an Intro or fill; else it's going to be an ending one and so be it ...
	for (int v = 0; v < 8; v++)
	{
		if ((variation[v].type == Topiary::VariationTypeFill) || (variation[v].type == Topiary::VariationTypeIntro) || variation[v].enabled)
		{
			previousSteadyVariation = v;
			v = 8;
		}
	}

} //initializePreviousSteadyVariation

///////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::outputNoteOn(int noteNumber)
{
	// always velocity 127!!! - output on the poolChannel of this note!!!
	// find the poolchannel; find the pool first
	int channel = -1;

	for (int i = 0; i < poolList.numItems; i++)
	{
		if (poolList.dataList[i].note == noteNumber)
		{
			channel = variation[variationSelected].poolChannel[poolList.dataList[i].pool-1];
			i = poolList.numItems;
		}
	}

	if (channel >= 0)
	{
		const GenericScopedLock<CriticalSection> myScopedLock(lockModel);
		MidiMessage msg = MidiMessage::noteOn(channel, noteNumber, (float) 1.0);
		modelEventBuffer.addEvent(msg, 0);
	}

} // outputNoteOn

///////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::outputNoteOff(int noteNumber)
{
	// output on the poolChannel of this note!!!
	// find the poolchannel; find the pool first
	int channel = -1;

	for (int i = 0; i < poolList.numItems; i++)
	{
		if (poolList.dataList[i].note == noteNumber)
		{
			channel = variation[variationSelected].poolChannel[poolList.dataList[i].pool-1];
			i = poolList.numItems;
		}
	}

	if (channel >= 0)
	{
		const GenericScopedLock<CriticalSection> myScopedLock(lockModel);
		MidiMessage msg = MidiMessage::noteOff(channel, noteNumber, (float) 1.0);
		modelEventBuffer.addEvent(msg, 0);
	}

} // outputNoteOff

///////////////////////////////////////////////////////////////////////////////////////

#include "../../Topiary/Source/TopiaryMidiLearnEditor.cpp.h"