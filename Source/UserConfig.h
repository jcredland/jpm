/*
  ==============================================================================

    UserConfig.h
    Created: 23 Dec 2015 7:12:07pm
    Author:  Adam Elemental

  ==============================================================================
*/

#ifndef USERCONFIG_H_INCLUDED
#define USERCONFIG_H_INCLUDED


/**
 * Stores downloaded files in a temporary loction and reuses those temporary
 * files when a download is requested.
*/
class UserConfig
{
public:
    UserConfig()
    {
        location = File::getSpecialLocation (File::userApplicationDataDirectory)
            .getChildFile ("jpm.config.json");

        location.createDirectory();
        
        if (location.exists())
        {
            // Load data
            String json = location.readEntireTextStream();
            properties = JSON::parse (json);
        }
    }
    
    
    
    
private:
    File location;
    var properties;

};


#endif  // USERCONFIG_H_INCLUDED
