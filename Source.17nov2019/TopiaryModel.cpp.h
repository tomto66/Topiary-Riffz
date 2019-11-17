/////////////////////////////////////////////////////////////////////////////
/*
This file is part of Topiary, Copyright Tom Tollenaere 2018-19.

Topiary is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Topiary is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Topiary. If not, see <https://www.gnu.org/licenses/>.
*/
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/*

This code has generic stuff that can be included in every Topiary model code.
It includes:
- Variation logic (that depends on variation variables defined in the actual 
  models (not in the generic TopiaryModel))

CAREFUL: needs a symbol TOPIARYMODEL to actually build!

*/
/////////////////////////////////////////////////////////////////////////////

#ifdef TOPIARYMODEL

void TOPIARYMODEL::setOverrideHostTransport(bool o)
{
	if (overrideHostTransport != o)
	{
		{   // block with a lock
			const GenericScopedLock<CriticalSection> myScopedLock(lockModel);
			if (o)
				overrideHostTransport = o;
			else
			{
				// make sure we are ready to accept host commands; there must be at least one variation enabled!
				bool ok = false;
				for (int v = 0; v < 8; v++)
					if (variation[v].enabled)
						ok = true;

				if (ok)
					overrideHostTransport = false;
				else
					Log("Host can only be in control if there is at least one enabled variation.", Topiary::LogType::Warning);
			}
		} // end lock

		broadcaster.sendActionMessage(MsgTransport);
		// careful here - if we just set the runstate to Stopped - it might have been stopped already and variables may be undefined
		// dirty hack below
		{
			const GenericScopedLock<CriticalSection> myScopedLock(lockModel);
			runState = -1000000; // to force a runstate stopped below!!!
		}
		setRunState(Topiary::Stopped);
		if (overrideHostTransport)
		{
			Log(String("Host transport overriden."), Topiary::LogType::Transport);
			recalcRealTime();
		}
		else
		{
			Log(String("Host transport in control."), Topiary::LogType::Transport);
			recalcRealTime();
		}

	}
} // setOverrideHostTransport

/////////////////////////////////////////////////////////////////////////////

void TOPIARYMODEL::setNumeratorDenominator(int nu, int de)
{
	
	if ((numerator != nu) || (denominator != de))
	{
#ifndef PRESETZ
		// check if there is data - if so do not allow meter changes!
#ifndef PADZ
		if (getNumPatterns() >0)
		{
			Log("Set meter first!", Topiary::LogType::Warning);
			Log("Meter change not allowed when patterndata available.", Topiary::LogType::Warning);
			return;
		}
#endif
#ifdef PADZ
		jassert(false);
#endif
#endif	
		const GenericScopedLock<CriticalSection> myScopedLock(lockModel);
		numerator = nu;
		denominator = de;
		recalcRealTime();
		Log(String("Signature set to ") + String(numerator) + String("/") + String(denominator), Topiary::LogType::Transport);
		broadcaster.sendActionMessage(MsgTransport);	
	}

} // setNumeratorDenominator

/////////////////////////////////////////////////////////////////////////////

void TOPIARYMODEL::getVariation(int& running, int& selected)
{
	running = variationRunning;
	selected = variationSelected;
	return;

} // getVariation

///////////////////////////////////////////////////////////////////////


void TOPIARYMODEL::setVariation(int n)
{
	jassert(n < 8);
	jassert(n >= 0);

	if (recordingMidi && (n!=variationSelected))
	{
		// do not allow switching of variations
		Log("Cannot switch variation during recording.", Topiary::LogType::Warning);
		return;
	}

 	if (variation[n].enabled == false)
	{
		// do not allow switching of disabled variations
		Log("Cannot switch to disabled variation.", Topiary::LogType::Warning);
		return;
	}
	if ((n != variationSelected) || (runState == Topiary::Stopped))
		// the || runState || is needed because we may need to re-set a waiting variation to non-waiting; in that case we want the update to happen otherwise the buttons stays orange
	{
		const GenericScopedLock<CriticalSection> myScopedLock(lockModel);

		variationSelected = n;
		if (runState == Topiary::Stopped)  // otherwise the switch will be done when running depending on the variation switch Q
			variationRunning = n;
		//Log(String("Variation ") + String(n + 1) + String(" selected."), Topiary::LogType::Variations);
		broadcaster.sendActionMessage(MsgVariationSelected);  // if the editor is there it will pick up the change in variation	
	}

#ifdef PRESETZ
	if ((runState != Topiary::Running) && variation[variationSelected].enabled)
		// we ALWAYS do this - even if the variation does not change (because we may wanna hit the variation button to reset it (presetz)
		// so even if WFFN is on - as long as we are waiting wo do an immediate output of the variation settings!
		outputVariationEvents(); // output the variation preset values!
#endif
} // setVariation

///////////////////////////////////////////////////////////////////////

void TOPIARYMODEL::getVariationEnables(bool enables[8])
{
	for (int i = 0; i < 8; i++)
		enables[i] = variation[i].enabled;

} // getVariationEnables

///////////////////////////////////////////////////////////////////////

int TOPIARYMODEL::getVariationLenInTicks(int v)
{
	return variation[v].pattern.patLenInTicks;

} // getVariationLenInTicks

///////////////////////////////////////////////////////////////////////

bool TOPIARYMODEL::getVariationEnabled(int v)
{
	return variation[v].enabled;

} // getVariationEnabled

///////////////////////////////////////////////////////////////////////

bool TOPIARYMODEL::processVariationSwitch() // called just before generateMidi - to see if a variation is changed, and if so whether to switch now (and make the switch)
{
	// next block goes from blockCursor to blockCursor+blockSize;
	// generation is at patternCursorOn within the pattern - which is variationRunning, not (yet) variationSelected
	// and both cursors go from 0 to the tickLength of that pattern
	// we also know that we are currently at rtCursor, and given rtCursor and patternCursorOn we can calculate when the current pattern started in realtime 
	
	//Logger::outputDebugString(String("Sel ")+String(variationSelected)+String(" Running ")+String(variationRunning));
	
	if (variationSelected == variationRunning) return false;

	if (blockCursor == 0)
	{
		runState = Topiary::Running;
		//Logger::outputDebugString("Starting - runstate is Armed but we need to go!");  // careful that the WFFN functonality keeps working!!!
		return true;
	}
	// in this case we want to start asap (unless we Q on the transport, but that is done in the call before this one!!!

//Logger::outputDebugString("Need to switch, now ???");
//Logger::outputDebugString("blockcursor "+String(blockCursor)+" NextRTcursor "+String(nextRTGenerationCursor));

	// first decide whether we should switch in this block
	// we are at measure/beat/tick overall; time and tick apply to the pattern we are in, but not measure within the pattern so we calculate that by using patternCursorO

	//Logger::outputDebugString("Need to switch, now ???  Something is going to happen next");
	calcMeasureBeat(); 
	int64 cursorToSwitch = 0;  // time the switch should happen; if  cursorToSwitch < blockCursor+blockSize then we know we want to switch;
	//Logger::outputDebugString("VariationStartQ " + String(variationStartQ));

	switch (variationStartQ)
	{
	case (Topiary::Immediate):
	{
		cursorToSwitch = blockCursor; // i.e. now
		//Logger::outputDebugString(String("IMMEDIATE Sel ") + String(variationSelected) + String(" Running ") + String(variationRunning));
		break;
	}
	case (Topiary::Measure):
	{
		// moment of next measure = blockCursor + time to next beat (ticksperquarter - tick) + #beats to go till end of measure			
		cursorToSwitch = (int64)(blockCursor + samplesPerTick * ((Topiary::TicksPerQuarter - tick - 1) + (numerator - beat - 1)* Topiary::TicksPerQuarter));
		//Logger::outputDebugString(String("MEASURE Sel ") + String(variationSelected) + String(" Running ") + String(variationRunning));
		break;
	}
	case (Topiary::Quarter):
	{
		// moment of next beat = blockCursor + time to next beat (ticksperquarter - tick) 
		cursorToSwitch = (int64)(blockCursor + samplesPerTick * ((Topiary::TicksPerQuarter - tick - 1)));
		//Logger::outputDebugString(String("QUARTER Sel ") + String(variationSelected) + String(" Running ") + String(variationRunning));
		break;
	}
	//case (Topiary::Half):
	//{
		// moment of next beat = blockCursor + time to next beat (ticksperquarter - tick) 
	//	cursorToSwitch = (int64)(blockCursor + samplesPerTick * ((Topiary::TicksPerQuarter - tick - 1) + Topiary::TicksPerQuarter));
	//	//Logger::outputDebugString(String("IMMEDIATE Half ") + String(variationSelected) + String(" Running ") + String(variationRunning));
	//	break;
	//}
	default: jassert(false);

	} // switch
	//Logger::outputDebugString(String("BlockCursor ") + String(blockCursor));
	//Logger::outputDebugString(String("CursorToSwitch ") + String(cursorToSwitch));

	//if ((nextRTGenerationCursor <= cursorToSwitch) && (cursorToSwitch < (blockCursor + blockSize))) 
	if (cursorToSwitch < (blockCursor + blockSize))
	{

		//Logger::outputDebugString("Need to switch, now ??? ----------------------------> yes we're sure");
		// we are sure we're switching variations now
		// so decide where in the variation we start - we need to set patternCursorOn (and we will set patternCursorOff to the same value)
		
		patternCursor = (int)floor(blockCursor / samplesPerTick) % getVariationLenInTicks(variationSelected);

		switch (switchVariation)
		{
		case (Topiary::SwitchFromStart): 
			patternCursorOffset = patternCursor;
			//Logger::outputDebugString("PatternCursorOffset in PROCESSVARIATIONSWITCH: " + String(patternCursorOffset));
			//Logger::outputDebugString("Blockcursor ; " + String(blockCursor));
			break;
		case (Topiary::SwitchWithinBeat): 
			patternCursorOffset = 0;
			patternCursor = patternCursor % Topiary::TicksPerQuarter;
			break;
		case (Topiary::SwitchWithinMeasure): 
			patternCursorOffset = 0;
			patternCursor = patternCursor % (Topiary::TicksPerQuarter*numerator);
			break;
		case (Topiary::SwitchWithinPattern):	
			patternCursorOffset = 0;
			patternCursor = patternCursor % (getVariationLenInTicks(variationSelected));
			break;
		}

		Log(String("Switch from variation ") + String(variationRunning) + String(" to ") + String(variationSelected), Topiary::LogType::Variations);
		variationRunning = variationSelected;

		if (variation[variationRunning].type == Topiary::VariationTypeSteady)
			previousSteadyVariation = variationRunning;

		broadcaster.sendActionMessage(MsgVariationSelected);

		return true;
	}
	else return false;

} // processVariationSwitch

///////////////////////////////////////////////////////////////////////

bool TOPIARYMODEL::switchingVariations()
{
	bool switching = (variationRunning != variationSelected);
	
#ifdef PRESETZ	
	// for Presetz, switching variations needs to trigger regeneration of the variations
	if (switching)
	{
		generateTransition(); // call regeneration of pattern 
	}
#endif

	return switching;

} // switchingVariations

///////////////////////////////////////////////////////////////////////

void TOPIARYMODEL::generateMidi(MidiBuffer* midiBuffer, MidiBuffer* recBuffer)
{ // main Generator

	const GenericScopedLock<CriticalSection> myScopedLock(lockModel);

	/*************************************************************************************************************************************************
	Uses a lot of model variables!  Summary of what is needed for what here

	variation[variationRunning].pattern;
	variation[variationRunning].currentPatternChild;
	variation[variationRunning].lenInTicks;
	variation[variationRunning].lenInMeasures;   DO WE NEED THIS, THINK NOT!

	int64 blockCursor;				// sampletime of start of current block
	int64 nextRTGenerationCursor;	// real time cursor of next event to generate
	int blockSize;					// size of block to generate
	int patternCursor;				// ticks where we are within the variation/pattern - if we do nothing it should still advance with blocksize/samplesPerTick

	**************************************************************************************************************************************************/

	int64 rtCursorFrom = 0;				// sampletime to start generating from
	int64 rtCursorTo = 0;				// we generate in principle till here
	int64 rtCursor = 0;					// where we are, between rtCursorFrom and rtCursorTo


	if ((runState == Topiary::Running) || (runState == Topiary::Ending) 
#ifndef PRESETZ
#ifndef PADZ
		|| (noteOffBuffer.bufferSize > 0)
#endif
#endif
		)
#ifdef PADZ
		jassert(false);
#endif
		if (blockCursor == 0)			// blockCursor is updated at end of generation!
		{
			patternCursorOffset = 0;
			rtCursorTo = rtCursorFrom + blockSize;
			nextRTGenerationCursor = 0;
			measure = 0;
			beat = 0;
			tick = 0;
			previousMeasure = -1;
#ifndef PRESETZ
#ifndef PADZ
			noteOffBuffer.bufferSize = 0;
#endif
#ifdef PADZ
				jassert(false);
#endif
#endif
		}
		else
		{
			rtCursorFrom = blockCursor;
			rtCursorTo = rtCursorFrom + blockSize;
			rtCursor = rtCursorFrom;
			calcMeasureBeat();
			patternCursor = (int)round((blockCursor / samplesPerTick) - patternCursorOffset); // not floor - that may lead to duplicate notes!!!

			//Logger::outputDebugString("Enter generateMidi with patternCursor " + String(patternCursor));
		}

	int parentLength; // in ticks
	MidiMessage msg;
	int noteChild = 0;
	int ticksTaken;
	int noteNumber;
	int length;
	int channel;
	int CC = 0;
	int value = 0;
	int64 timestamp;

#ifdef PRESETZ
	UNUSED(recBuffer)
#endif
#ifdef BEATZ


		////////////////////////////////////////
		// Record logic here
		////////////////////////////////////////

#define RECBUFFERSIZE 100
		MidiMessage recordBuffer[RECBUFFERSIZE];
	int samplePos = 0;

	if (recordingMidi && ((runState == Topiary::Running) || (runState == Topiary::Ending)))
	{
		TopiaryVariation* varpat = nullptr;
		int rememberPatternCursor = patternCursor;

		for (MidiBuffer::Iterator it(*recBuffer); it.getNextEvent(msg, samplePos);)
		{
			if (msg.isNoteOn() || msg.isNoteOff())
			{
				if (varpat == nullptr)
					varpat = &(variation[variationRunning].pattern);
				varpat->add();
				int nIndex = varpat->numItems - 1;
				varpat->dataList[nIndex].ID = -1; // dummy
				varpat->dataList[nIndex].note = msg.getNoteNumber();
				varpat->dataList[nIndex].velocity = msg.getVelocity();
				varpat->dataList[nIndex].channel = msg.getChannel();
				varpat->dataList[nIndex].length = -1; // so we know not to generate a noteOff event (because the noteOff will follow)

				if (msg.isNoteOn())
					varpat->dataList[nIndex].midiType = Topiary::MidiType::NoteOn;
				else
					varpat->dataList[nIndex].midiType = Topiary::MidiType::NoteOff;

				int64 cursorInTicks = (int64)floor(blockCursor / samplesPerTick) + (int)(samplePos / samplesPerTick);
				cursorInTicks = cursorInTicks % variation[variationSelected].pattern.patLenInTicks;

				varpat->dataList[nIndex].timestamp = (int)cursorInTicks;
				//Logger::outputDebugString("RECORDING");
			} // noteOn or noteOff
		} // iterator over recordBuffer

		// reset patternchild to nullptr so the rest of the logic can restart (because we may have messed with patternchild
		variation[variationRunning].currentPatternChild = 0;
		patternCursor = rememberPatternCursor;

		// sort by timstamp
		if (varpat != nullptr) // we did record something
			varpat->sortByTimestamp(false); // keep the -1 IDs se we know later which events got inserted

	}

	////////////////////////////////////////
	// End of record logic
	////////////////////////////////////////

#endif

	//Logger::outputDebugString("Generate midi; patcur" + String(patternCursor));
	//Logger::outputDebugString("next RTcursor " + String(nextRTGenerationCursor));

	if ((runState == Topiary::Running) || (runState == Topiary::Ending))
	{
		jassert(beat >= 0);
		jassert(measure >= 0);

		TopiaryVariation* parentPattern = &(variation[variationRunning].pattern);
		parentLength = parentPattern->patLenInTicks;

		noteChild = variation[variationRunning].currentPatternChild;

		int nextPatternCursor;  // patternCursor is global and remembers the last one we generated

		patternCursor = (int)patternCursor % parentLength;

		//Logger::outputDebugString("Next note on to generate afer current tick " + String(patternCursor));

		bool walk;

		if (!(variation[variationRunning].type == Topiary::VariationTypeEnd) || ((variation[variationRunning].type == Topiary::VariationTypeEnd) && !variation[variationRunning].ended))
		{
			walk = walkToTick(parentPattern, noteChild, patternCursor);
		}
		else
			walk = false; // meaning an ending variation and ended


		//Logger::outputDebugString(String("Walk: ") + String((int)walk));
		
		if (walk)
		{
			// set patternCursors where we are now, so the offsets in sample time are correct
			patternCursor = (int)patternCursor % parentLength;  //////// because rtCursors are multi loops !!!!

			//Logger::outputDebugString("PatternCursor = " + String(patternCursor));
			//Logger::outputDebugString("DEBUGPatternCursor = " + String(DEBUGpatCursor));
			//Logger::outputDebugString("PatternCursorOffset = " + String(patternCursorOffset));
			//Logger::outputDebugString("Blockcursor ; " + String(blockCursor));


			while (rtCursor < rtCursorTo)
			{
				auto rememberChild = noteChild;

				nextPatternCursor = parentPattern->dataList[noteChild].timestamp;

				ticksTaken = nextPatternCursor - patternCursor;  // ticks taken in this timeframe
				if (ticksTaken < 0)
				{
					Logger::outputDebugString("PatternCursor looped over end");

					if (variation[variationRunning].type == Topiary::VariationTypeEnd)
					{
						// this is an ending variation and it has now ended - do nothing (but keep running)
						//Log("Ended ---------------------------------------", Topiary::LogType::Variations);
						variation[variationRunning].ended = true;
						rtCursor = rtCursorTo; // prevent next if to pass so nothing further is generated
					}
					else if (runState == Topiary::Ending)
					{
						setRunState(Topiary::TransportRunState::Stopped); 
						//Log("Ended ---------------------------------------", Topiary::LogType::Variations);
						rtCursor = rtCursorTo; // prevent next if to pass so nothing further is generated

					}

					ticksTaken += parentLength;
					jassert(ticksTaken >= 0);
				}

				if ((rtCursor + (int64)(ticksTaken*samplesPerTick)) < rtCursorTo)
				{
					//Logger::outputDebugString("GENERATING A NOTE " + String(noteChild->getIntAttribute("midiType")) + "------------>" + String(noteChild->getIntAttribute("Note"))+" at timest "+ String(noteChild->getIntAttribute("Timestamp")));
					//Logger::outputDebugString(String("Next Patcursor ") + String(nextPatternCursor));

					////// GENERATE MIDI EVENT
					int midiType = parentPattern->dataList[noteChild].midiType;

					int pMeasure = (int)floor(parentPattern->dataList[noteChild].timestamp / (numerator * Topiary::TicksPerQuarter));
					if (pMeasure != previousMeasure)
					{
						// regenerate a measure
#ifndef PRESETZ
						if (!recordingMidi) // because we do not want to loose the recorded notes!
						{
#ifdef PADZ
							jassert(false);  // do generateVariation and delete these IFDEFS
#endif
#ifndef PADZ
							if (previousMeasure == -1)
								generateVariation(variationRunning, 0); // make sure we do the first one, always !!!
							else
								generateVariation(variationRunning, pMeasure+1); // the others we do one ahead to preven early notes from dropping
#endif
							// DO WE NEED TO WALK HERE - I THINK WE CAN AVOID IT !!!! 
							noteChild = 0;
							walkToTick(parentPattern, noteChild, patternCursor);
							midiType = parentPattern->dataList[noteChild].midiType;
						}
#endif
						previousMeasure = pMeasure;
					}

					if ((variation[variationRunning].type == Topiary::VariationTypeFill) && (variationRunning == variationSelected))
					{
						// if variationRunning != variationSelected that takes precedence !!!!
						// if we are just now entering the last beat of the last measure, set variationSelected to the previously running steady variation; if none, pick the first one
						// Careful -  user should not set Q var switch to immediate because in that case it will switch early!
						// (if folks use that they will probably have Q Var Switch at measure anyway)
						// alternatively, check whether this is the last run over the variation before we might switch (based on 2*blocksize or something)
						if (((patternCursor + (int)(2 * blockSize / samplesPerTick)) >= parentPattern->patLenInTicks) || (noteChild == (parentPattern->numItems - 1)))
						{
							variationSelected = previousSteadyVariation;  // initialized at run start (tellModelToRun) and maintained in processVariationSwitch
						}
					}
					else if (variation[variationRunning].type == Topiary::VariationTypeIntro)
					{
						// if variationRunning != variationSelected that takes precedence !!!!
						// if we are entering the last beat of the last measure of the variation, set variationSelected to the first steady variation; if none, stay where  we are
						// Careful - user should not set Q var switch to immediate because in that case it will switch early!
						// (if folks use that they will probably have Q Var Switch at measure anyway)
						// alternatively, check whether this is the last run over the variation before we might switch (based on 2*blocksize or something)

						if (((patternCursor + (int)(2 * blockSize / samplesPerTick)) >= parentPattern->patLenInTicks) || (noteChild == (parentPattern->numItems - 1)))
						{
							int newVariation = variationRunning;
							for (int v = 0; v < 8; v++)
							{
								// limit search to enabled variations
								if ((variation[v].type == Topiary::VariationTypeSteady) || variation[v].enabled)
								{
									newVariation = v;
									v = 8;
								}
								variationSelected = newVariation;
							}
						}

					}


					if ((midiType == Topiary::MidiType::NoteOn) || (midiType == Topiary::MidiType::NoteOff))
					{ // Generate note on/off; ignore other midi events for now
						length = parentPattern->dataList[noteChild].length;
						noteNumber = parentPattern->dataList[noteChild].note;
						channel = parentPattern->dataList[noteChild].channel;
						timestamp = (int)(rtCursor - rtCursorFrom);

						if (midiType == Topiary::MidiType::NoteOn)
							msg = MidiMessage::noteOn(channel, noteNumber, (float)parentPattern->dataList[noteChild].velocity / 128);
						else
							msg = MidiMessage::noteOff(channel, noteNumber, (float)0.0);

						// DEBUG LOGIC !!!!!!!!!
						// outputting the pattern tick values + the tick value in the generated pattern
						// int64 cursorInTicks = (int64)floor( (rtCursor + (int64)(ticksTaken*samplesPerTick) ) / samplesPerTick    );
						// now do that modulo the patternlenght in ticks
						// cursorInTicks = cursorInTicks % parentLength;
						// Logger::outputDebugString("Generated note at realtime pat tick " + String(cursorInTicks) + " / tick in pattern " + String(noteChildOn->getStringAttribute("Timestamp")));
						/////////////////////////

						/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						// schedule noteOff event if this is noteOn and len != -1; otherwise it's a recording event and the noteOff will follow
						/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef PRESETZ
#ifndef PADZ
						if ((midiType == Topiary::MidiType::NoteOn) && (parentPattern->dataList[noteChild].length != -1))
						{
							noteOffBuffer.push(channel, noteNumber, rtCursor + (int64)((ticksTaken + parentPattern->dataList[noteChild].length) * samplesPerTick));
							jassert(channel > 0);
						}
#endif
#ifdef PADZ
						jassert(false);
#endif
#endif
						///////////////////////////////////
						// OUTPUT THE NOTE EVENT
						///////////////////////////////////

						midiBuffer->addEvent(msg, (int)timestamp);
						if (logMidiOut)
							logMidi(false, msg);
					}
					else if (midiType == Topiary::MidiType::CC)
					{
						channel = parentPattern->dataList[noteChild].channel;
						CC = parentPattern->dataList[noteChild].CC;
						value = parentPattern->dataList[noteChild].value;
						msg = MidiMessage::controllerEvent(channel, CC, value);
						timestamp = (int)(rtCursor - rtCursorFrom);
						processCC(msg); // make sure model is updated with new cc values

						///////////////////////////////////
						// OUTPUT THE CC EVENT
						///////////////////////////////////

						midiBuffer->addEvent(msg, (int)timestamp);
						if (logMidiOut)
							logMidi(false, msg);
					}
					else if (midiType == Topiary::MidiType::NOP)
					{
						//jassert(false); // but this is really fine
					}

					patternCursor = nextPatternCursor;  // that is the tick of the event we just generated
					if (patternCursor >= parentLength) patternCursor = patternCursor - parentLength;

					nextTick(parentPattern, noteChild);

					//Logger::outputDebugString(String("NOTE ON --------"));
					rtCursor = rtCursor + (int64)(ticksTaken * samplesPerTick);

					jassert((rtCursor - rtCursorFrom) >= 0);
					jassert((rtCursor - rtCursorFrom) < blockSize);

					if (noteChild == rememberChild)
					{
						// can only happen in recording in empty pattern; note OFF but no note ON yet
						// force end
						rtCursor = rtCursor + blockSize;
					}
					//Logger::outputDebugString(String("nxtcursor ") + String(nextRTGenerationCursor));

				}  // generated a note 
				else
				{
					// done for now; next event is over rtCursorTo
					// let's place ourselves ready for the next round
					// so either walkOn and we have the next On note ready
					// or walkOff and next Off note ready as well
					// main goal is to set nextPatternCursor!

					//Logger::outputDebugString(String(" ++++++++++++++ done +++++++++++++++++++++"));

					nextPatternCursor = parentPattern->dataList[noteChild].timestamp;
					ticksTaken = nextPatternCursor - patternCursor;
					if (ticksTaken < 0)
					{
						ticksTaken += parentLength;
						//Logger::outputDebugString(String("Running over end of pattern!!!"));
					}

					patternCursor = nextPatternCursor;
					if (patternCursor >= parentLength) patternCursor = patternCursor - parentLength;

					// we set rtCursor at the time of the next event; will possibly break out of the loop if not within this block

					rtCursor = rtCursorTo; // force break out of the loop
					nextRTGenerationCursor = rtCursorFrom + (int64)(ticksTaken * samplesPerTick);
					//int nextTick = (int) (nextRTGenerationCursor / samplesPerTick) % parentLength;
					//Logger::outputDebugString("Next tick to generate (off nextRTcursor): " + String(nextTick));
				}  // end loop over from --> to
			}
		}
		else
		{
			// walk did not find a note to generate
			// so pattern is empty or no fitting note events found
			if ((noteChild == 0))
			{
				// make sure our cursors keep running
				// 		  nextRTGenerationCursor = rtCursorTo + 1;
				nextRTGenerationCursor = rtCursorTo;
				patternCursor = +(int)(blockSize / samplesPerTick);

			}
		} // pattern is empty
	} // if Running or Ending

#ifndef PRESETZ
	// output any noteOff events scheduled in this block - this can be called even when ended!!!
#ifndef PADZ
	if (noteOffBuffer.bufferSize > 0)

		while (noteOffBuffer.firstTimeStamp() < (blockCursor + blockSize))
		{
			noteOffBuffer.pop(channel, noteNumber, timestamp);
			msg = MidiMessage::noteOff(channel, noteNumber, (float)0.0);
			midiBuffer->addEvent(msg, (int)(timestamp - blockCursor));
			jassert((timestamp - blockCursor) >= 0);
			jassert((timestamp - blockCursor) < blockSize);

			if (logMidiOut)
				logMidi(false, msg);
		}
#endif
#ifdef PADZ
	jassert(false);
#endif
	
#endif

#ifdef PADZ
	jassert(false); // check the below
#endif

	if ((runState == Topiary::Running) || (runState == Topiary::Ending) 
#ifndef PRESETZ
#ifndef PADZ
		|| (noteOffBuffer.bufferSize > 0)
#endif
#endif
		)
	{
		blockCursor = blockCursor + blockSize;
		calcMeasureBeat();
	}
} // generateMidi

///////////////////////////////////////////////////////////////////////

bool TOPIARYMODEL::walkToTick(TopiaryVariation* parent, int& childIndex, int toTick)
{
	// Find the first child on or after this tick, starting from current child; 
	// Caller has to make sure that child is child of parent, or pass nullptr to initialize
	// Return false if pattern is empty or nothing to do within the blockSize; if empty then child == nullptr; 
	// This loops around the pattern! 
	// returns childIndex when called from recording (before record we set nextPatternChild to nullptr to make this correct)
	// returns correct prevChild if there is one

	int childTick;

	if (childIndex == 0)
	{
		if (parent->numItems == 0)
		{
			return false; // empty pattern!
		}
	}
	childTick = parent->dataList[childIndex].timestamp;

	while (childTick < toTick)
	{   // as long as our child is behind time we're looking for

		childIndex++;
		if (childIndex == parent->numItems) break; // there are no events afther the given time
		childTick = parent->dataList[childIndex].timestamp;
	}

	if (childIndex == parent->numItems) // wrap around the pattern
	{
		childIndex = 0;
		return true;
	}
	return true;

} // walkToTick

///////////////////////////////////////////////////////////////////////

void TOPIARYMODEL::nextTick(TopiaryVariation* parent, int& childIndex)
{	// assert that parent has at least 1 child of each!!! (do a walk to a tick first!)
	// this one loops around the pattern

	childIndex++;
	if (childIndex == parent->numItems)
	{
		childIndex = 0;
		//Logger::outputDebugString(String("-------- LOOPING OVER END OFF PATTERN ==================== "));
	}

}  // nextTick
#endif