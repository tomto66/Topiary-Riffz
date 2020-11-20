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

#include "TopiaryRiffz.h"
#include "../../Topiary/Source/Model/TopiaryPatternList.h"
#include "../../Topiary/Source/Model/TopiaryPattern.h"
#include "../../Topiary/Source/Model/TopiaryVariation.h"
#include "../../Topiary/Source/Model/TopiaryNoteOffBuffer.h"
#include "../../Topiary/Source/Model/TopiaryKeytracker.h"
#include "../../Topiary/Source/Components/TopiaryMidiLearnEditor.h"

#include "NoteAssignmentList.h"

#define MAXPATTERNSINVARIATION 8

class TopiaryRiffzModel : public TopiaryModel
{
public:

	enum SwingQButtonIds
	{
		SwingQ4 = 3004,
		SwingQ8 = 3008,
		SwingQRadioID = 3000
	};

	
	TopiaryRiffzModel();
	~TopiaryRiffzModel();

	void quantize(int p, int ticks);

	void getPatterns(String pats[MAXNOPATTERNS]);
	
	int getPatternLengthInMeasures(int i);  // needed in variationComponent for validating pool length values;
	void setPatternLengthInMeasures(int i, int l);
	void deletePattern(int i);
	void addPattern();
	void duplicatePattern(int p); 
	void clearPattern(int p);

	void setPatternLength(int p, int l, bool keepTail);
	void deleteNote(int p, int n);				// deletes the note with ID n from pattern p
	void getNote(int p, int ID, int& note, int &velocity, int &timestamp, int &length, int &midiType, int &value);  // get note with id ID from pattern p
	void addNote(int p, int n, int v, int l, int t);	// adds note n in pattern p, with velocity v at time t
	void addAT(int p, int at, int timestamp);
	void addPitch(int p, int value, int timestamp);
	void addCC(int p, int CC, int value, int timestamp);

	void deleteAllNotes(int p, int n);  // deletes all occurrence of note n (id of event) in the pattern

	void setLatch(bool l);
	bool getLatch(); 
	void setOutputChannel(int c);
	int getOutputChannel();
	void getKeyRange(int& f, int& t);

	void setKeyRange(int f, int t);

	void regenerateVariationsForPattern(int p);
	
	void setGMDrumMapLabels();
	bool insertPatternFromFile(int patternIndex, bool overload);
	void validateTableEdit(int p, XmlElement* child, String attribute); // see if user edits to this attribute make sense and do housekeeping

	void saveStateToMemoryBlock(MemoryBlock& destData) override;
	void restoreStateFromMemoryBlock(const void* data, int sizeInBytes) override;
	bool processVariationSwitch() override;
	bool switchingVariations() override;
	void initializeVariationsForRunning() override;
	void setRunState(int n) override;
	void initializePreviousSteadyVariation();
	void generateMidi(MidiBuffer* midiBuffer, MidiBuffer* recBuffer) override;
	
	////// Variations

	void setVariation(int i) override;
	void getVariation(int& running, int& selected);
	void getVariationEnables(bool enables[8]);
	bool getVariationEnabled(int v);
	int getVariationLenInTicks(int v);

	void getVariationDefinition(int i, bool& enabled, String& vname, int& type);   // pass variation Definition on to VariationComponent in editor
	void setVariationDefinition(int i, bool enabled, String vname, int type);      // set variation Definition parameters from editor; return false if we try to disable the last enabled variation
	bool validateVariationDefinition(int i, String vname);
	void setRandomizeNotes(int v, bool enable, int value);
	void getRandomizeNotes(int v, bool &enable,  int &value);
	void setSwing(int v, bool enable, int value);
	void getSwing(int v, bool &enable, int &value);
	void setRandomizeVelocity(int v, bool enable, int value, bool plus, bool min);
	void getRandomizeVelocity(int v, bool &enable, int &value, bool &plus, bool &min);
	void setRandomizeTiming(int v, bool enable, int value, bool plus, bool min);
	void getRandomizeTiming(int v, bool &enable,  int &value, bool &plus, bool &min);
	void setRandomizeLength(int v, bool enable, int value, bool plus, bool min);
	void getRandomizeLength(int v, bool &enable,  int &value, bool &plus, bool &min);
	void saveNoteAssignment(int v, int n, int o, int p);
	void getNoteAssignment(int v, int i, String& n, int& o, int& patternId);
	void deleteNoteAssignment(int v, int i);
	//int getVariationLenInTicks(int v, int p); 

	NoteAssignmentList* getNoteAssignment(int v);
	void setSwingQ(int v, int q);
	int getSwingQ(int v);
	void setNoteOrder(int n);
	int getNoteOrder();

	void generateVariation(int v, int measureToGenerate); // calls the next one below for all patterns
	void generateVariation(int v, int p, int measureToGenerate); // Generates the variation;
	void generateAllVariations(int measureToGenerate);

	void setOverrideHostTransport(bool o) override;
	void setNumeratorDenominator(int nu, int de) override;

	struct PatternLookUp
	{
		int patternId;             // ID in the patternlist
		int patternInVariationId;  // ID of placeholder of generated pattern in the variation
		/*
		If note N uses pattern P, then the pattern is generated in pattern[T] in this variation,
		where T = patternVariationId for the patternLookUp records that has patternId P
		Unused patternLookUps have patternID == -1 and patternInVariationId == -1
		*/
	};

	struct Variation {

		int lenInMeasures;
		TopiaryVariation pattern[MAXPATTERNSINVARIATION];			// pattern  events in the variation
		
		int type;
		bool ended;

		bool enabled = false;
		String name = "";

		// note occurrence randomization
		bool randomizeNotes;
		int randomizeNotesValue;

		// swing
		bool swing;
		int swingValue;

		// velocity
		bool randomizeVelocity;
		int velocityValue;
		bool velocityPlus;
		bool velocityMin;

		// timing
		bool randomizeTiming;
		int timingValue;
		bool timingPlus;
		bool timingMin;

		// length
		bool randomizeLength;
		int lengthValue;
		bool lengthPlus;
		bool lengthMin;
		int swingQ;

		NoteAssignmentList noteAssignmentList;
		PatternLookUp patternLookUp[MAXPATTERNSINVARIATION];  
	};

	void swapVariation(int from, int to) override; 
	void copyVariation(int from, int to) override; 
	bool midiLearn(MidiMessage m); // called by processor
	void record(bool b) override; // tells model to record or not; at end of recording it processes the new notes
	void processMidiRecording() override; // add recorded events to the pattern
	void maintainParentPattern();

	TopiaryPatternList* getPatternList();
	TopiaryPattern* getPattern(int p);
	int getNumPatterns();

	bool walkToTick(TopiaryVariation* parent, int& childIndex, int toTick);
	void nextTick(TopiaryVariation* parent, int& childIndex);

	void outputNoteOn(int noteNumber);
	void outputNoteOff(int noteNumber);
	TopiaryKeytracker keytracker;
	TopiaryVariation* parentPattern; // maintained by void maintainParentattern in processvariationSwitch

	int keyRangeFrom = 0;
	int keyRangeTo = 127;
#define NUMBEROFQUANTIZERS 10

private:
	TopiaryPatternList patternList;
	TopiaryPattern patternData[MAXNOPATTERNS];
	Variation variation[8];  	// struct to hold variation detail
	TopiaryNoteOffBuffer noteOffBuffer;
	
	int outputChannel = 1;		// output of plugin
	bool latch = true;		// keep playing last note
	

	//////////////////////////////////////////////////////////////////////////////////////////////////

#include "..//..//Topiary/Source/Model/LoadMidiPattern.cpp.h"	
#include "../../Topiary/Source/Model/Swing.cpp.h"

	
	//////////////////////////////////////////////////////////////////////////////////////////////////

	bool patternAssigned(int v, int p)
	{
		// see if pattern p is already assigned in variation[v].patternLookUP
		// if so return the assignment, if not return -1
		bool assigned = false;

		for (int i = 0; i < MAXPATTERNSINVARIATION; i++)
			if (variation[v].patternLookUp[i].patternId == p)
				assigned = true;

		return assigned;

	} // patternAssignment

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void redoPatternLookup(int v)
	{
		// redo the patternLookup because of new assignments, for variation v

		// reset everything
		for (int j = 0; j < MAXPATTERNSINVARIATION; j++)
		{
			variation[v].patternLookUp[j].patternId = -1;
			variation[v].patternLookUp[j].patternInVariationId = -1;
		}

		int n = 0;

		// loop over the patternassignments and update n for each newly used pattern inthere

		for (int i = 0; i < variation[v].noteAssignmentList.numItems; i++)
		{
			if (!patternAssigned(v, variation[v].noteAssignmentList.dataList[i].patternId))
			{
				// make a new assignment in patternLookup
				variation[v].patternLookUp[n].patternId = variation[v].noteAssignmentList.dataList[i].patternId;
				variation[v].patternLookUp[n].patternInVariationId = n;
				n ++;
			}
		}

	} // redoPatternLookup

	//////////////////////////////////////////////////////////////////////////////////////////////////

	int findPatternInVariation(int v, int p)
	{
		// p = patternId in variation.pattern[]
		// v is variation
		// returns index in pattern[] to use to generate/run pattern p
		// find that by looking in patternLookUp
		int j;
		for (j = 0; j < MAXPATTERNSINVARIATION; j++)
		{
			if (variation[v].patternLookUp[j].patternInVariationId == p)
				return variation[v].patternLookUp[j].patternId;
		}
		jassert(j < MAXPATTERNSINVARIATION); // corrupt data!
		return 0;

	}  // findPatternInVariation

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void addParametersToModel()
	{
		model.reset(new XmlElement("Riffz"));
		auto patternListData = new XmlElement("PatternList");
		model->addChildElement(patternListData);
		// this one will have a series of <PatternData>
		patternList.addToModel(patternListData);

		auto patternDataa = new XmlElement("PatternData");
		// this one will have a series of <Pattern>
		model->addChildElement(patternDataa);
		for (int p = 0; p < patternList.numItems; p++)
			patternData[p].addToModel(patternDataa);

		auto parameters = new XmlElement("Parameters");
		model->addChildElement(parameters);

		addToModel(parameters, name, "name");
		addToModel(parameters, BPM, "BPM");
		addToModel(parameters, numerator, "numerator");
		addToModel(parameters, denominator, "denominator");
		addToModel(parameters, switchVariation, "switchVariation");
		addToModel(parameters, runStopQ, "runStopQ");
		addToModel(parameters, variationStartQ, "variationStartQ");
		addToModel(parameters, WFFN, "WFFN");
		addToModel(parameters, name, "name");
		addToModel(parameters, latch, "latch");
		addToModel(parameters, outputChannel, "outputChannel");
		addToModel(parameters, keyRangeFrom, "keyRangeFrom");
		addToModel(parameters, keyRangeTo, "keyRangeTo");
		addToModel(parameters, keytracker.noteOrder, "noteOrder");

		addToModel(parameters, overrideHostTransport, "overrideHostTransport");
		addToModel(parameters, notePassThrough, "notePassThrough");

		addToModel(parameters, logMidiIn, "logMidiIn");
		addToModel(parameters, logMidiOut, "logMidiOut");
		addToModel(parameters, logDebug, "logDebug");
		addToModel(parameters, logTransport, "logTransport");
		addToModel(parameters, logVariations, "logVariations");
		addToModel(parameters, logInfo, "logInfo");
		addToModel(parameters, filePath, "filePath");

		addToModel(parameters, variationSwitchChannel, "variationSwitchChannel");
		addToModel(parameters, ccVariationSwitching, "ccVariationSwitching");

		for (int i = 0; i < 8; i++) {

			// Variations

			addToModel(parameters, variation[i].lenInMeasures, "lenInMeasures", i);

			addToModel(parameters, variation[i].name, "variationName", i);
			addToModel(parameters, variation[i].enabled, "variationEnabled", i);
			addToModel(parameters, variation[i].type, "variationType", i);

			addToModel(parameters, variation[i].randomizeNotes, "randomizeNotes", i);
			addToModel(parameters, variation[i].randomizeNotesValue, "randomizeNotesValue", i);

			addToModel(parameters, variation[i].swing, "swing", i);
			addToModel(parameters, variation[i].swingValue, "swingValue", i);

			addToModel(parameters, variation[i].randomizeVelocity, "randomizeVelocity", i);
			addToModel(parameters, variation[i].velocityValue, "velocityValue", i);
			addToModel(parameters, variation[i].velocityPlus, "velocityPlus", i);
			addToModel(parameters, variation[i].velocityMin, "velocityMin", i);

			addToModel(parameters, variation[i].randomizeTiming, "randomizeTiming", i);
			addToModel(parameters, variation[i].timingValue, "timingValue", i);
			addToModel(parameters, variation[i].timingPlus, "timingPlus", i);
			addToModel(parameters, variation[i].timingMin, "timingMin", i);

			addToModel(parameters, variation[i].randomizeLength, "randomizeLength", i);
			addToModel(parameters, variation[i].lengthValue, "lengthValue", i);
			addToModel(parameters, variation[i].lengthPlus, "lengthPlus", i);
			addToModel(parameters, variation[i].lengthMin, "lengthMin", i);
			

			addToModel(parameters, variation[i].swingQ, "swingQ", i);


			auto noteAssignmentData = new XmlElement("noteAssignments");
			noteAssignmentData->setAttribute("variation", i);
			// this one will have a series of <Pattern>
			parameters->addChildElement(noteAssignmentData);

			variation[i].noteAssignmentList.addToModel(noteAssignmentData); // will add the full list

			// automation
			addToModel(parameters, variationSwitch[i], "variationSwitch", i);

		} // end loop over variations

	} // addParametersToModel

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void restoreParametersToModel()
	{

		overrideHostTransport = true; // otherwise we might get very weird effects if the host were running
		setRunState(Topiary::Stopped);

		auto child = model->getFirstChildElement();
		jassert(child->getTagName().equalsIgnoreCase("PatternList"));
		patternList.getFromModel(child);

		child = child->getNextElement();
		jassert(child->getTagName().equalsIgnoreCase("PatternData"));

		// load all patternData
		XmlElement* pchild = child->getFirstChildElement();
		int p = 0;
		while (pchild != nullptr)
		{
			patternData[p].getFromModel(pchild);
			pchild = pchild->getNextElement();
			p++;
		}

		child = child->getNextElement();
		bool rememberOverride = true; // we do not want to set that right away!
		jassert(child->getTagName().equalsIgnoreCase("Parameters"));
		
		while (child != nullptr)
		{
				String tagName = child->getTagName();

				if (tagName.compare("Parameters") == 0)
				{
					auto parameter = child->getFirstChildElement();
					while (parameter != nullptr)
					{
						String parameterName = parameter->getStringAttribute("Name");
						if (parameterName.compare("name") == 0)	name = parameter->getStringAttribute("Value");
						else if (parameterName.compare("BPM") == 0) BPM = parameter->getIntAttribute("Value");
						else if (parameterName.compare("numerator") == 0) numerator = parameter->getIntAttribute("Value");
						else if (parameterName.compare("overrideHostTransport") == 0) rememberOverride = (bool)parameter->getIntAttribute("Value");
						else if (parameterName.compare("denominator") == 0) denominator = parameter->getIntAttribute("Value");

						else if (parameterName.compare("switchVariation") == 0) switchVariation = parameter->getIntAttribute("Value");
						else if (parameterName.compare("runStopQ") == 0) runStopQ = parameter->getIntAttribute("Value");
						else if (parameterName.compare("noteOrder") == 0) keytracker.noteOrder = parameter->getIntAttribute("Value");
						else if (parameterName.compare("variationStartQ") == 0) variationStartQ = parameter->getIntAttribute("Value");
						else if (parameterName.compare("name") == 0) name = parameter->getStringAttribute("Value");

						else if (parameterName.compare("keyRangeFrom") == 0) keyRangeFrom = parameter->getIntAttribute("Value");
						else if (parameterName.compare("keyRangeTo") == 0) keyRangeTo = parameter->getIntAttribute("Value");
						else if (parameterName.compare("latch") == 0) latch = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("outputChannel") == 0)outputChannel = parameter->getIntAttribute("Value");

						else if (parameterName.compare("WFFN") == 0)	WFFN = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("notePassThrough") == 0) 	notePassThrough = parameter->getBoolAttribute("Value");

						else if (parameterName.compare("logMidiIn") == 0) logMidiIn = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("logMidiOut") == 0)	logMidiOut = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("logDebug") == 0)	logDebug = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("logTransport") == 0)	logTransport = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("logVariations") == 0) logVariations = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("logInfo") == 0)	logInfo = parameter->getBoolAttribute("Value");

						else if (parameterName.compare("filePath") == 0)	filePath = parameter->getStringAttribute("Value");

						//if (parameterName.compare("lenInTicks") == 0) variation[parameter->getIntAttribute("Index")].lenInTicks = parameter->getIntAttribute("Value");
						else if (parameterName.compare("lenInMeasures") == 0) variation[parameter->getIntAttribute("Index")].lenInMeasures = parameter->getIntAttribute("Value");
						else if (parameterName.compare("variationEnabled") == 0) variation[parameter->getIntAttribute("Index")].enabled = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("variationType") == 0) variation[parameter->getIntAttribute("Index")].type = parameter->getIntAttribute("Value");

						else if (parameterName.compare("variationName") == 0) variation[parameter->getIntAttribute("Index")].name = parameter->getStringAttribute("Value");
						else if (parameterName.compare("randomizeNotes") == 0) variation[parameter->getIntAttribute("Index")].randomizeNotes = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("randomizeNotesValue") == 0) variation[parameter->getIntAttribute("Index")].randomizeNotesValue = parameter->getIntAttribute("Value");

						else if (parameterName.compare("swing") == 0) variation[parameter->getIntAttribute("Index")].swing = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("swingValue") == 0) variation[parameter->getIntAttribute("Index")].swingValue = parameter->getIntAttribute("Value");

						else if (parameterName.compare("randomizeVelocity") == 0) variation[parameter->getIntAttribute("Index")].randomizeVelocity = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("velocityValue") == 0) variation[parameter->getIntAttribute("Index")].velocityValue = parameter->getIntAttribute("Value");
						else if (parameterName.compare("velocityPlus") == 0) variation[parameter->getIntAttribute("Index")].velocityPlus = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("velocityMin") == 0) variation[parameter->getIntAttribute("Index")].velocityMin = parameter->getBoolAttribute("Value");

						else if (parameterName.compare("randomizeTiming") == 0) variation[parameter->getIntAttribute("Index")].randomizeTiming = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("timingValue") == 0) variation[parameter->getIntAttribute("Index")].timingValue = parameter->getIntAttribute("Value");
						else if (parameterName.compare("timingPlus") == 0) variation[parameter->getIntAttribute("Index")].timingPlus = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("timingMin") == 0) variation[parameter->getIntAttribute("Index")].timingMin = parameter->getBoolAttribute("Value");

						else if (parameterName.compare("randomizeLength") == 0) variation[parameter->getIntAttribute("Index")].randomizeLength = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("lengthValue") == 0) variation[parameter->getIntAttribute("Index")].lengthValue = parameter->getIntAttribute("Value");
						else if (parameterName.compare("lengthPlus") == 0) variation[parameter->getIntAttribute("Index")].lengthPlus = parameter->getBoolAttribute("Value");
						else if (parameterName.compare("lengthMin") == 0) variation[parameter->getIntAttribute("Index")].lengthMin = parameter->getBoolAttribute("Value");

						else if (parameterName.compare("swingQ") == 0) variation[parameter->getIntAttribute("Index")].swingQ = parameter->getIntAttribute("Value");

						// automation
						else if (parameterName.compare("variationSwitch") == 0)  variationSwitch[parameter->getIntAttribute("Index")] = parameter->getIntAttribute("Value");
						else if (parameterName.compare("ccVariationSwitching") == 0)  ccVariationSwitching = (bool)parameter->getIntAttribute("Value");
						else if (parameterName.compare("variationSwitchChannel") == 0)  variationSwitchChannel = parameter->getIntAttribute("Value");


						// note assignment lists

						else if (parameter->getTagName().equalsIgnoreCase("noteAssignments"))
						{
							int v = parameter->getIntAttribute("variation");
							variation[v].noteAssignmentList.getFromModel(parameter); // will retrieve the whole list
							//  reconstruct note label and pattern name
							for (int n = 0; n < variation[v].noteAssignmentList.numItems; n++)
							{
								int note = variation[v].noteAssignmentList.dataList[n].note;
								int patternId = variation[v].noteAssignmentList.dataList[n].patternId;
								variation[v].noteAssignmentList.dataList[n].noteLabel = MidiMessage::getMidiNoteName(note, true, true, 5);;
								variation[v].noteAssignmentList.dataList[n].patternName = patternList.dataList[patternId].name;
							}
							
						}
						else
							jassert(false); // something unknown read

						parameter = parameter->getNextElement();
					}
					break;
				} // if tagname == parameters
				
				child = child->getNextElement();

		} // foreach parameters

		

		// if there are no patterns; all variations need to be disabled!!!

		if (patternList.getNumItems() == 0)
		{
				for (int i = 0; i < 8; i++)
					variation[i].enabled = false;
		}

		for (int i = 0; i < 8; i++)
		{
			redoPatternLookup(i);
		}

		variationSelected = 0;
		setRunState(Topiary::Stopped);
		setOverrideHostTransport(rememberOverride);

		// generate the variations
		generateAllVariations(-1);

		// inform editor
		broadcaster.sendActionMessage(MsgLoad); // tell everyone we've just loaded something (table headers need to be re-set
		broadcaster.sendActionMessage(MsgTransport);
		broadcaster.sendActionMessage(MsgLog);
		broadcaster.sendActionMessage(MsgPatternList);
		broadcaster.sendActionMessage(MsgPattern);
		broadcaster.sendActionMessage(MsgVariationEnables);		// so that if needed variationbuttons are disabled/enabled
		broadcaster.sendActionMessage(MsgVariationDefinition);	// inform editor of variation settings;
		broadcaster.sendActionMessage(MsgVariationAutomation);	// inform editor of variation automation settings;	

	} // restoreParametersToModel


	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Generator stuff	
	//////////////////////////////////////////////////////////////////////////////////////////////////


	void initializeVariation(int i)
	{
		// full initialize to nothing!!!
		jassert(false);
		//variation[i].patternToUse = -1;				// index in patterndata
		
		// be sure to keep the actual pattern here!!! if ever changed it will get overwritten by generateVariation

		jassert(false); // more stuff to initialize
		variation[i].lenInMeasures = 0;
		for (int j = 0; j < MAXPATTERNSINVARIATION; j++)
		{
			variation[i].pattern[j].patLenInTicks = 0;
			variation[i].patternLookUp[j].patternId = -1;
			variation[i].patternLookUp[j].patternInVariationId = -1;
		}

		variation[i].type = Topiary::VariationTypeSteady;				// indicates that once pattern played, we no longer generate notes! (but we keep running (status Ended) till done

		variation[i].name = "Variation " + String(i + 1);
		variation[i].enabled = false;
		

	} // initializeVariation

	///////////////////////////////////////////////////////////////////////

	void initializeVariations()
	{
jassert(false);
		// called as soon as first pattern is added; initializes all variations to use that pattern
		for (int i = 0; i < 8; ++i)
		{
			initializeVariation(i);
		}
	} // initializeVariations

	///////////////////////////////////////////////////////////////////////

	void initializePatternToVariation(int p, int v)
	{
		// to call when setting a pattern, 
		// uses indexes in the data structures; not variation or pattern numbers!
		// if pattern number == -1 disable the variation
jassert(false);
		if (p == -1) variation[v].enabled = false;

		//variation[v].patternToUse = p;
		
		// don't forget to set the length!!!
		
		if (p != -1)
		{
			variation[v].lenInMeasures = getPatternLengthInMeasures(p);
			//variation[v].pattern.patLenInTicks = patternData[variation[v].patternToUse].patLenInTicks;
		}

	} // initializePatternToVariations


	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopiaryRiffzModel)
};


