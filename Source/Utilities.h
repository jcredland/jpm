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

class JpmFatalExcepton
{
public:
    JpmFatalExcepton (const String& error_, const String& debugInfo_)
        :
        error (error_), debugInfo (debugInfo_)
    {}

    String error;
    String debugInfo;
};

inline void printWarning (const String& s)
{
    std::cout << "jpm **   : " << s << std::endl;
}

inline void printInfo (const String& s)
{
    std::cout << "jpm      : " << s << std::endl;
}

inline void printError (const String& s)
{
    std::cout << "jpm error: " << s << std::endl;
}



#endif  // UTILITIES_H_INCLUDED
