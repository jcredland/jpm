/*
  ==============================================================================

    Repository.h
    Created: 7 Dec 2015 1:32:45pm
    Author:  Jim

  ==============================================================================
*/

#ifndef REPOSITORY_H_INCLUDED
#define REPOSITORY_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Module.h"
#include <iostream>

/** Holds an index of where to find specific modules.  We may want more than one of these in the end. */
class Directory
{
public:
	Module getModuleByName(const String & moduleName)
	{
		if (moduleName == "juce_core")
		{
			Module m;
			m.setName("juce_core"); 
			m.setVersion("3.1.1"); 
			m.setPath("https://github.com/julianstorer/JUCE/archive/"); 
			m.setSubPath("JUCE-3.1.1/modules/juce_core");
			m.setSource("GitHub"); 
			return m;
		}

		return Module(); /* invalid module */
	}
};



#endif  // REPOSITORY_H_INCLUDED
