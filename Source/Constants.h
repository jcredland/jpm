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
    constexpr static char databaseUrl[] { "http://juce.cloudant.com/" };
#else
    constexpr static char databaseUrl[] { "https://juce.cloudant.com/" };
#endif
    constexpr static char registry[] { "registry" };
    constexpr static char users[] { "_users" };
    constexpr static char databaseKeyReadOnly[] { "illikedwerstochemortooth" };
    constexpr static char databasePasswordReadOnly[] { "17b7e80852bb3837ce1187734e5f88debef1848e" };
};



#endif  // CONSTANTS_H_INCLUDED
