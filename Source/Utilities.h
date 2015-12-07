/*
  ==============================================================================

    Utilities.h
    Created: 7 Dec 2015 8:56:49pm
    Author:  Jim Credland

  ==============================================================================
*/

#ifndef UTILITIES_H_INCLUDED
#define UTILITIES_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

inline void printWarning(const String & s)
{
    std::cout << "** " << s << std::endl;
}

inline void printInfo(const String & s)
{
    std::cout << "   " << s << std::endl;
}

inline void printError(const String & s)
{
    std::cout << "!! " << s << std::endl;
}



#endif  // UTILITIES_H_INCLUDED
