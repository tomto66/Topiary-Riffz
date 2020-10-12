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

#include "TopiaryRiffzModel.h"
#include "Build.h"

// following has std model code that can be included (cannot be in TopiaryModel because of variable definitions)
#include"../../Topiary/Source/Model/TopiaryModel.cpp.h"
#include "../../Topiary/Source/Model/TopiaryPattern.cpp.h"

void TopiaryRiffzModel::saveStateToMemoryBlock(MemoryBlock& destData)
{
	addParametersToModel();  // this adds an XML element "Parameters" to the model
	AudioProcessor::copyXmlToBinary(*model, destData);
	
}

/////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::restoreStateFromMemoryBlock(const void* data, int sizeInBytes)
{
	model = AudioProcessor::getXmlFromBinary(data, sizeInBytes);
	restoreParametersToModel();
	

}

/////////////////////////////////////////////////////////////////////////

TopiaryRiffzModel::TopiaryRiffzModel()
{
	Log(String("Topiary Riffz V ") + String(xstr(JucePlugin_Version)) + String(" - ") + String(BUILD_DATE) 
#ifdef _DEBUG
		+ String("D")
#endif
		+ String("."), Topiary::LogType::License);
	Log(String("(C) Tom Tollenaere 2019-2020."), Topiary::LogType::License);
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


	name = "New Riffz";
	// give some of the children yourself as model

	for (int p = 0; p < 8; p++)
		patternData[p].setModel(this);

	/////////////////////////////////////
	// variations initialisation
	/////////////////////////////////////

	for (int i = 0; i < 8; ++i)
	{
		
		//variation[i].patternToUse = -1;				// index in patterndata
		variation[i].noteAssignmentList.setRiffzModel(this);
		variation[i].lenInMeasures = 0;
		for (int j=0; j < MAXPATTERNSINVARIATION; j++)
			variation[i].pattern[j].patLenInTicks = 0;
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

		variation[i].randomizeLength = false;
		variation[i].lengthValue = 0;
		
		variation[i].swingQ = TopiaryRiffzModel::SwingQButtonIds::SwingQ4;
		
	}

	/////////////////////////////////////
	// PatternData initialization
	/////////////////////////////////////


	for (int i = 0; i < MAXNOPATTERNS; ++i)
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
	noteOrder = NoteOrder::Lowest;


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
	jassert(false);
	/*
	// if there are variations that use this pattern, they need to be re-initialized (send in -1 in the initialize code)
	for (int j = 0; j < 8; j++)
	{
		
		//if (variation[j].patternToUse == deletePattern)
		//	initializePatternToVariation(-1, j);
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
	*/
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
		success = loadMidiPattern(f, patternIndex, patternMeasures, lenInTicks);

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

			bool deassigned = false;

			// if any variations use this pattern, unassign them 
			for (int v = 0; v < 8; v++)
			{
				jassert(false);
				/*
				if (variation[v].patternToUse == patternIndex)
				{
					initializePatternToVariation(-1, v);
					Log("Variation " + String(v + 1) + " deassigned from this pattern.", Topiary::LogType::Warning);
					broadcaster.sendActionMessage(MsgWarning);  // warn the user in the header
					deassigned = true;
				}*/
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

void TopiaryRiffzModel::setLatch(bool l)
{
	latch = l;
}

bool TopiaryRiffzModel::getLatch()
{
	return latch;
}

void TopiaryRiffzModel::setOutputChannel(int c)
{
	outputChannel = c;
}

int TopiaryRiffzModel::getOutputChannel()
{
	return outputChannel;
}

void TopiaryRiffzModel::setNoteOrder(int n)
{
	noteOrder = n;
}

int TopiaryRiffzModel::getNoteOrder()
{
	return noteOrder;
}
void TopiaryRiffzModel::setKeyRange(int f, int t)
{
	keyRangeFrom = f;
	keyRangeTo = t;
}

void TopiaryRiffzModel::getKeyRange(int& f, int& t)
{
	f = keyRangeFrom;
	t = keyRangeTo;
}

///////////////////////////////////////////////////////////////////////
//  VARIATIONS
///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::initializeVariationsForRunning()
{
	// careful; this code is different in various plugins - don't think it's generic!!!
	for (int v = 0; v < 8; v++)
	{
		variation[v].ended = false;
		//variation[v].currentPatternChild = 0;
	}
} // initializeVariationsForRunning

///////////////////////////////////////////////////////////////////////
// Variation Parameters
///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getVariationDefinition(int i, bool& enabled, String& vname,  int& type)
{
	jassert(i < 8); // goes from 0-7
	vname = variation[i].name;
	enabled = variation[i].enabled;
	type = variation[i].type;

} // getVariationDefinition

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setVariationDefinition(int i, bool enabled, String vname, int type)
{
	// write to model
	// assume the data has been validated first!
	// make sure that overrideHost is set to TRUE if there are no enabled variations!!!

	jassert(i < 8); // goes from 0-7
	if (getNumPatterns() == 0) return;  // should not really happen unless @ initialization - but in that case we shouldn't write to the model

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

	
	broadcaster.sendActionMessage(MsgVariationDefinition);
	broadcaster.sendActionMessage(MsgVariationEnables);  // may need to update the enable buttons

}  // setVariationDefinition

//////////////////////////////////////////////////////////////////////

bool TopiaryRiffzModel::validateVariationDefinition(int i, String vname)
{
	// return false if parameters do not make sense
	// refuse to enable a variation if there are no patterns
	// refuse to enable a variation if the pool settings make no sense and do not match e.g. length of pattern
	// if patternId == -1 check if it was -1 in the model; if so give is a pass; otherwise complain that pattern needs to be selected

	jassert(i < 8); // goes from 0-7

	if (getNumPatterns() == 0) return false;  // in this case the variations tab should be disabled anyway

	bool refuse = false;

	if (!name.compare("")) {
		Log("Variation needs a name.", Topiary::LogType::Warning);
		refuse = true;
	}

	if (refuse) Log("Current values will not be remembered!", Topiary::LogType::Warning);

	return refuse;

} // validateVariationDefinition

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setRandomizeNotes(int v, bool enable, int value)
{
	variation[v].randomizeNotes = enable;
	variation[v].randomizeNotesValue = value;
	
	generateVariation(v, -1);
	
} // getRandomNotes

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getRandomizeNotes(int v, bool &enable, int &value)
{
	enable = variation[v].randomizeNotes;
	value = variation[v].randomizeNotesValue;
	
} // setRandomNotes

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setRandomizeLength(int v, bool enable, int value, bool plus, bool min)
{
	variation[v].randomizeLength = enable;
	variation[v].lengthValue = value;
	variation[v].lengthPlus = plus;
	variation[v].lengthMin = min;
	generateVariation(v, -1);

} // getRandomNotes

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getRandomizeLength(int v, bool& enable, int& value, bool& plus, bool& min)
{
	enable = variation[v].randomizeLength;
	value = variation[v].lengthValue;
	plus = variation[v].lengthPlus;
	min = variation[v].lengthMin;

} // setRandomLength

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setSwing(int v, bool enable, int value)
{
	variation[v].swing = enable;
	variation[v].swingValue = value;
	generateVariation(v, -1);

} // getSwing

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getSwing(int v, bool &enable, int &value)
{
	enable = variation[v].swing;
	value = variation[v].swingValue;
	
} // getSwing

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setRandomizeVelocity(int v, bool enable, int value, bool plus, bool min)
{
	variation[v].randomizeVelocity = enable;
	variation[v].velocityValue = value;
	variation[v].velocityPlus = plus;
	variation[v].velocityMin = min;	

	generateVariation(v, -1);

} // getRandomizeVelocity

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getRandomizeVelocity(int v, bool &enable, int &value, bool &plus, bool &min)
{
	enable = variation[v].randomizeVelocity;
	value = variation[v].velocityValue;
	plus = variation[v].velocityPlus;
	min = variation[v].velocityMin;

} // getRandomizeVelocity

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::setRandomizeTiming(int v, bool enable, int value, bool plus, bool min)
{
	variation[v].randomizeTiming = enable;
	variation[v].timingValue = value;
	variation[v].timingPlus = plus;
	variation[v].timingMin = min;

	generateVariation(v, -1);

} // setRandomizeTiming

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getRandomizeTiming(int v, bool &enable, int &value, bool &plus, bool &min)
{
	enable = variation[v].randomizeTiming;
	value = variation[v].timingValue;
	plus = variation[v].timingPlus;
	min = variation[v].timingMin;


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

NoteAssignmentList* TopiaryRiffzModel::getNoteAssignment(int v)
{
	return &(variation[v].noteAssignmentList);
}

///////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getPatterns(String pats[MAXNOPATTERNS])
{
	for (int i=0; i < MAXNOPATTERNS; i++)
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::saveNoteAssignment(int v, int n, int o, int p)
{
	// variation v, note n, offest o and patter numer p
	// assumes all has been validated!

	// check if this note already exists, if so, overwrite, if not make a new one
	int noteExistsIndex = -1;
	for (int i = 0; i < variation[v].noteAssignmentList.getNumItems(); i++)
	{
		if (variation[v].noteAssignmentList.dataList[i].note == n)
		{
			noteExistsIndex = i;
			Log("Previous note assignment overwritten", Topiary::Warning);
		}
	}

	if (noteExistsIndex == -1)
	{
		variation[v].noteAssignmentList.add();
		noteExistsIndex = variation[v].noteAssignmentList.getNumItems() - 1;
	}

	// now assign all values
	variation[v].noteAssignmentList.dataList[noteExistsIndex].note = n;
	variation[v].noteAssignmentList.dataList[noteExistsIndex].noteLabel = MidiMessage::getMidiNoteName(n, true, true, 5);;
	variation[v].noteAssignmentList.dataList[noteExistsIndex].offset = o;
	variation[v].noteAssignmentList.dataList[noteExistsIndex].patternId = p;
	variation[v].noteAssignmentList.dataList[noteExistsIndex].patternName = patternList.dataList[p].name;
	redoPatternLookup(v);

	broadcaster.sendActionMessage(MsgNoteAssignment);
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getNoteAssignment(int v, int i, String& n, int& o, int& patternId)
{
	n = variation[v].noteAssignmentList.dataList[i].noteLabel;
	patternId = variation[v].noteAssignmentList.dataList[i].patternId;
	o = variation[v].noteAssignmentList.dataList[i].offset;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::deleteNoteAssignment(int v, int i)
{
	variation[v].noteAssignmentList.del(i);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::generateVariation(int v, int measureToGenerate)
{
	// calls generateVaration(v, p, measureToGenerate) for each pattern
	
	for (int p = 0; p < MAXNOTEASSIGNMENTSPERVARIATION; p++)
		if (variation[v].patternLookUp->patternInVariationId != -1)
			generateVariation(v, variation[v].patternLookUp->patternInVariationId, measureToGenerate);

} // generateVariation

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::generateVariation(int v, int p, int measureToGenerate)
{
	// (re)generates variation[v].pattern[p]

	jassert((v < 8) && (v >= 0));

	if (!variation[v].enabled)
		return;

	if (measureToGenerate == -1)
	{
		// meaning we regererate the lot
		// reset numItems
		variation[v].pattern[p].numItems = 0;
	}

	// set every event in the variation to miditype::NOP

	TopiaryVariation* var = &(variation[v].pattern[p]); 

	// find out which pattern to use in the variation; we are talking source patterns in the pattern structure, not in the variation itself
	int patternToUse = 	findPatternInVariation(v, p);  

	TopiaryPattern* pat = &(patternData[patternToUse]);
	if (measureToGenerate == -1)
	{
		// make sure it's initialized properly - only done from editor or @ load
		var->numItems = pat->numItems;
		for (int j = 0; j < pat->numItems; j++)
		{
			var->dataList[j].ID = j + 1;
			var->dataList[j].timestamp = pat->dataList[j].timestamp;
			int measre = pat->dataList[v].timestamp % (numerator * Topiary::TicksPerQuarter);
			if ((measre == measureToGenerate) || (measureToGenerate == -1))
				var->dataList[j].midiType = Topiary::MidiType::NOP;
		}

	}
	else if (measureToGenerate == patternList.dataList[patternToUse].measures) // we ran over end of pattern
		measureToGenerate = 0;


	for (int j = 0; j < pat->numItems; j++)
	{
		int measre = (int) pat->dataList[j].timestamp / (numerator * Topiary::TicksPerQuarter);
		if ((measre == measureToGenerate) || (measureToGenerate == -1))
			var->dataList[j].midiType = Topiary::MidiType::NOP;
	}

	////////////////////////////
	// GENERATE NOTES & EVENTS
	////////////////////////////
	
	Logger::outputDebugString("Generating notes & events");

	Random randomizer;
	
	var->patLenInTicks = pat->patLenInTicks; // make sure length is correct

	int note;
	int vIndex; 

	
	for (int pIndex = 0; pIndex < pat->numItems; pIndex++)
	{
		if (((*pat).dataList[pIndex].measure == measureToGenerate) || (measureToGenerate == -1))
		{
			bool doNote = true;
			vIndex = var->findID(pIndex + 1);

			note = (*pat).dataList[pIndex].note;
			int midiType = (*pat).dataList[pIndex].midiType;

			if (midiType == Topiary::NoteOn)
			{
				var->dataList[vIndex].note = note;
				// note randomization logic
				
				vIndex = var->findID(pIndex + 1);

				if (variation[v].randomizeNotes)
				{
					float rnd = randomizer.nextFloat();
					// decide whether we're generating this one or not
					if (rnd > ((float)variation[v].randomizeNotesValue / 100))
					{
						doNote = false;
					}

				};

				// if (doNote) we will generate a note on event; if not it will be a NOP event
				if (doNote)
					var->dataList[vIndex].midiType = Topiary::MidiType::NoteOn;
				else
					var->dataList[vIndex].midiType = Topiary::MidiType::NOP;

			// length randomisation
			var->dataList[vIndex].length = pat->dataList[pIndex].length;

			if (variation[v].randomizeLength)
			{
				int len = pat->dataList[vIndex].length;
				float rnd;
				int direction;
				if (variation[v].lengthPlus && variation[v].lengthMin)
				{
					rnd = randomizer.nextFloat();
					if (rnd > 0.5)
						direction = 1;
					else
						direction = -1;
				}
				else if (variation[v].velocityPlus)
					direction = 1;
				else direction = -1;

				rnd = randomizer.nextFloat();
				//int debug = direction * rnd * 128 * variation[v].velocityValue /100;
				len = len + (int)(direction * rnd * 128 * len / 100);   // originally / 100 but I want more of an effect

				/// make sure it does not run over length of the pattern
				/// int length = var->dataList[vIndex].length;
				if (len >= var->patLenInTicks)
				{
					len = var->patLenInTicks - 1;
				}
				else if (len < 0)
					len = 1;

				var->dataList[vIndex].length = len;

			} // end randomize length

			var->dataList[vIndex].velocity = pat->dataList[pIndex].velocity;
			int timestamp = pat->dataList[pIndex].timestamp;
			var->dataList[vIndex].timestamp = timestamp;
			if (doNote)
				if (variation[v].randomizeVelocity && (variation[v].velocityPlus || variation[v].velocityMin))
				{
					int vel = pat->dataList[pIndex].velocity;
					float rnd;
					int direction;
					if (variation[v].velocityPlus && variation[v].velocityMin)
					{
						rnd = randomizer.nextFloat();
						if (rnd > 0.5)
							direction = 1;
						else
							direction = -1;
					}
					else if (variation[v].velocityPlus)
						direction = 1;
					else direction = -1;

					rnd = randomizer.nextFloat();
					
					vel = vel + (int)(direction * rnd * 128 * variation[v].velocityValue / 50);   // originally / 100 but I want more of an effect

					if (variation[v].randomizeVelocity)
						vel = vel + variation[v].velocityValue;

					if (vel > 127) vel = 127;
					else if (vel < 0) vel = 0;
					var->dataList[vIndex].velocity = vel;

				} // velocity randomization


			if (variation[v].swing && doNote)
			{
				// recalc the timestamp, based on the swing lookup table
				//Logger::outputDebugString("Timestamp pre " + String(timestamp));
				int base;

				//double debugFirst;
				//int debugSecond;
				//int debugThird;
				if (variation[v].swingQ == Topiary::SwingQButtonIds::SwingQ8)
				{
					base = ((int)floor(timestamp / (Topiary::TicksPerQuarter / 2))) * (int)(Topiary::TicksPerQuarter / 2);
					//debugFirst = floor(timestamp / (Topiary::TicksPerQuarter / 2));
					//debugSecond = (int)(Topiary::TicksPerQuarter / 2);
					//debugThird = (int)floor(timestamp / (Topiary::TicksPerQuarter / 2));

				}
				else
					base = ((int)floor(timestamp / Topiary::TicksPerQuarter)) * Topiary::TicksPerQuarter;

				//Logger::outputDebugString("Remainder pre " + String(timestamp-base));

				int remainder = swing(timestamp - base, variation[v].swingValue, variation[v].swingQ);
				var->dataList[vIndex].timestamp = base + remainder;

				//Logger::outputDebugString("Remainder post" + String (remainder));
				//Logger::outputDebugString("New Timestamp " + String(base + remainder) + " (DIFF: " + String (timestamp-base-remainder));

				jassert((remainder) <= Topiary::TicksPerQuarter);
				jassert((base + remainder) >= 0);

			} // swing
		} // end of note specific randomization stuff, except for timing
		else if (midiType == Topiary::CC)
		{
			var->dataList[vIndex].midiType = Topiary::CC;
			var->dataList[vIndex].note = pat->dataList[pIndex].note;
			var->dataList[vIndex].value = pat->dataList[pIndex].value;
		}
		else if (midiType == Topiary::AfterTouch)
		{ 
			var->dataList[vIndex].midiType = Topiary::AfterTouch;
			var->dataList[vIndex].value = pat->dataList[pIndex].value;
		}
		else if (midiType == Topiary::Pitch)
		{
			var->dataList[vIndex].midiType = Topiary::Pitch;
			var->dataList[vIndex].value = pat->dataList[pIndex].value;
		}
		else jassert(false);
				
		// we apply timing randomization AFTER possible swing !!!
		if ((doNote) || (midiType != Topiary::NoteOn))
					if (variation[v].randomizeTiming && (variation[v].timingPlus || variation[v].timingMin))
					{
						int timestamp = var->dataList[vIndex].timestamp;
						float rnd;
						int direction = -1;
						if (variation[v].velocityPlus && variation[v].velocityMin)
						{
							rnd = randomizer.nextFloat();
							if (rnd > 0.5)
								direction = 1;
						}
						else if (variation[v].velocityPlus)
							direction = 1;

						rnd = randomizer.nextFloat();
						//int debug = direction * rnd * Topiary::TicksPerQuarter * variation[v].timingValue /100;
						timestamp = timestamp + (int)(direction * rnd * Topiary::TicksPerQuarter * variation[v].timingValue / 800);
						if (timestamp < 0) timestamp = 0;

						if (timestamp > (var->patLenInTicks - 1)) timestamp = var->patLenInTicks - 1; // lenInTicks -1 because we need time for the note off event
						var->dataList[vIndex].timestamp = timestamp;

						if (midiType == Topiary::NoteOn)
						{
							// make sure we do not run over the patternlenght with note off
							// and only for notes
							int length = var->dataList[vIndex].length;
							if ((length + timestamp) >= var->patLenInTicks)
							{
								length = var->patLenInTicks - timestamp - 1;
								var->dataList[vIndex].length = length;
								jassert(length > 0);
							}
						}
						
					} // end randomize timing

		//Logger::outputDebugString("Generating Note " + String(var->dataList[vIndex].note) + " timestamp " + String(var->dataList[vIndex].timestamp));
		//auto debug = 0;

			
		} // end check for measureToGenerate
	}  // for children in original pattern

	////////// end note generation
	
	
	

	//Logger::outputDebugString("SORTED ------------------------");
	variation[v].pattern[p].sortByTimestamp();
	
	Logger::outputDebugString("------------------------");
	for (int j = 0; j < variation[v].pattern[p].numItems; j++)
	{
		if (variation[v].pattern[p].dataList[j].midiType == Topiary::NoteOn)
				Logger::outputDebugString("<" + String(j) + "> <ID" + String(variation[v].pattern[p].dataList[j].ID)+"> Note: " + String(variation[v].pattern[p].dataList[j].note) + 
					" timestamp " + String(variation[v].pattern[p].dataList[j].timestamp) + 
					" len " + String(variation[v].pattern[p].dataList[j].length) +
					" midiType " + String(variation[v].pattern[p].dataList[j].midiType));
		else if (variation[v].pattern[p].dataList[j].midiType == Topiary::CC)
			Logger::outputDebugString("<" + String(j) + "> <ID" + String(variation[v].pattern[p].dataList[j].ID) + "> CC: " + String(variation[v].pattern[p].dataList[j].note) +
				" timestamp " + String(variation[v].pattern[p].dataList[j].timestamp) +
				" value " + String(variation[v].pattern[p].dataList[j].value) );
		else if (variation[v].pattern[p].dataList[j].midiType == Topiary::AfterTouch)
			Logger::outputDebugString("<" + String(j) + "> <ID" + String(variation[v].pattern[p].dataList[j].ID) + "> AT " + 
				" timestamp " + String(variation[v].pattern[p].dataList[j].timestamp) +
				" value " + String(variation[v].pattern[p].dataList[j].value));
		else if (variation[v].pattern[p].dataList[j].midiType == Topiary::Pitch)
			Logger::outputDebugString("<" + String(j) + "> <ID" + String(variation[v].pattern[p].dataList[j].ID) + "> Pitch: " + 
				" timestamp " + String(variation[v].pattern[p].dataList[j].timestamp) +
				" value " + String(variation[v].pattern[p].dataList[j].value));
	}
	Logger::outputDebugString("------------------------");
	
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
		if (variation[v].enabled)
			for (int pat=0; pat< MAXNOTEASSIGNMENTSPERVARIATION; pat++)
				if (variation[v].patternLookUp[pat].patternId == p)
									generateVariation(v, variation[v].patternLookUp[pat].patternId, -1);
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
	
} // deleteNote

////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::addNote(int p, int n, int v, int l, int timestamp) 
{   
	int measuree = (int)(timestamp / (denominator*Topiary::TicksPerQuarter));
	int t = timestamp % (denominator*Topiary::TicksPerQuarter);
	int beatt = (int)(t / Topiary::TicksPerQuarter);
	t = t % Topiary::TicksPerQuarter;
	patternData[p].addNote(measuree, beatt, t, timestamp, n, l, v);
	patternData[p].sortByTimestamp(); // will create the ID
	broadcaster.sendActionMessage(MsgPattern);
	regenerateVariationsForPattern(p);

}  // addNote

////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::addAT(int p, int at, int timestamp)
{
	int measuree = (int)(timestamp / (denominator * Topiary::TicksPerQuarter));
	int t = timestamp % (denominator * Topiary::TicksPerQuarter);
	int beatt = (int)(t / Topiary::TicksPerQuarter);
	t = t % Topiary::TicksPerQuarter;
	patternData[p].addAT(measuree, beatt, t, timestamp, at);
	patternData[p].sortByTimestamp(); // will create the ID
	broadcaster.sendActionMessage(MsgPattern);
	regenerateVariationsForPattern(p);

}  // addAT

////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::addCC(int p, int CC, int value, int timestamp)
{
	int measuree = (int)(timestamp / (denominator * Topiary::TicksPerQuarter));
	int t = timestamp % (denominator * Topiary::TicksPerQuarter);
	int beatt = (int)(t / Topiary::TicksPerQuarter);
	t = t % Topiary::TicksPerQuarter;
	patternData[p].addCC(measuree, beatt, t, timestamp, CC, value);
	patternData[p].sortByTimestamp(); // will create the ID
	broadcaster.sendActionMessage(MsgPattern);
	regenerateVariationsForPattern(p);

}  // addCC

////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::addPitch(int p, int value, int timestamp)
{
	int measuree = (int)(timestamp / (denominator * Topiary::TicksPerQuarter));
	int t = timestamp % (denominator * Topiary::TicksPerQuarter);
	int beatt = (int)(t / Topiary::TicksPerQuarter);
	t = t % Topiary::TicksPerQuarter;
	patternData[p].addPitch(measuree, beatt, t, timestamp, value);
	patternData[p].sortByTimestamp(); // will create the ID
	broadcaster.sendActionMessage(MsgPattern);
	regenerateVariationsForPattern(p);

}  // addPitch


////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::getNote(int p, int ID, int& note, int &velocity, int &timestamp, int &length, int &midiType, int &value)  // get note with id ID from pattern p
{
	// get note with id ID from pattern p
	note = patternData[p].dataList[ID].note;
	velocity = patternData[p].dataList[ID].velocity;
	length = patternData[p].dataList[ID].length;
	timestamp = patternData[p].dataList[ID].timestamp;
	midiType = patternData[p].dataList[ID].midiType;
	value = patternData[p].dataList[ID].value;

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

///////////////////////////////////////////////////////////////////////////////////////

int TopiaryRiffzModel::getNumPatterns()
{
	// needed for TopiaryModelIncludes
	return patternList.numItems;
}

///////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::swapVariation(int from, int to)
{
	jassert((from < 8) && (from >= 0));
	jassert((to < 8) && (to >= 0));

	jassert(false); // code not done
	{
		const GenericScopedLock<CriticalSection> myScopedLock(lockModel);

		//intSwap(variation[from].patternToUse, variation[to].patternToUse);
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
		

		intSwap(variation[from].swingQ, variation[to].swingQ);

		
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

	jassert(false); // code note done!

	{
		const GenericScopedLock<CriticalSection> myScopedLock(lockModel);

		//variation[to].patternToUse = variation[from].patternToUse;
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
		
		variation[to].swingQ = variation[from].swingQ;

		//variation[to].currentPatternChild = 0;
		
	} // end of lock scope

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

	if (b) {
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
	
	jassert(false);
	/*
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

	generateAllVariations(-1);
	// inform pattern tab
	broadcaster.sendActionMessage(MsgPattern);
	*/
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
		const GenericScopedLock<CriticalSection> myScopedLock(lockModel);
		MidiMessage msg = MidiMessage::noteOn(outputChannel, noteNumber, (float) 1.0);
		modelEventBuffer.addEvent(msg, 0);

} // outputNoteOn

///////////////////////////////////////////////////////////////////////////////////////

void TopiaryRiffzModel::outputNoteOff(int noteNumber)
{
		const GenericScopedLock<CriticalSection> myScopedLock(lockModel);
		MidiMessage msg = MidiMessage::noteOff(outputChannel, noteNumber, (float) 1.0);
		modelEventBuffer.addEvent(msg, 0);
	
} // outputNoteOff

///////////////////////////////////////////////////////////////////////////////////////

#include "../../Topiary/Source/Components/TopiaryMidiLearnEditor.cpp.h"