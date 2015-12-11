/*
  ==============================================================================

    Constants.h
    Created: 11 Dec 2015 3:07:36pm
    Author:  jim

  ==============================================================================
*/

#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

namespace Constants
{
    /** How soon things expire from the download cache. */
    static const int cacheRefreshMinutes { 60 };

    constexpr static char versionString[] { "0.01" };

#if JUCE_LINUX
    constexpr static char databaseUrl[] { "http://codegarden.cloudant.com/jpm/" };
#else
    constexpr static char databaseUrl[] { "https://codegarden.cloudant.com/jpm/" };
#endif
    constexpr static char databaseKeyReadOnly[] { "romenglentiouldissionged" };
    constexpr static char databasePasswordReadOnly[] { "cbe04fcf61bd85eb946e31ce0c310adabc2986b4" };
};



#endif  // CONSTANTS_H_INCLUDED
