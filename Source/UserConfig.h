/*
  ==============================================================================

    UserConfig.h
    Created: 23 Dec 2015 7:12:07pm
    Author:  Adam Elemental

  ==============================================================================
*/

#ifndef USERCONFIG_H_INCLUDED
#define USERCONFIG_H_INCLUDED

#include "Utilities.h"

/**
 * Manages user details and
*/
class UserConfig
{
public:
    UserConfig()
    {
        configFile = File::getSpecialLocation (File::userApplicationDataDirectory)
            .getChildFile ("jpm.config.json");

        configFile.createDirectory();
        
        DBG (configFile.getFullPathName());
        
        if (configFile.existsAsFile())
        {
            // Load data
            String json = configFile.loadFileAsString();
            properties = JSON::parse (json); // Should probably do some error checking here
        }
    }
    
    void saveUser(const String& username, const String& password, const String& email)
    {
        DynamicObject* obj;
        if (!properties.isVoid() && properties.isObject())
        {
            obj = properties.getDynamicObject();
        }
        else
        {
            obj = new DynamicObject();
        }
        obj->setProperty("username", username);
        obj->setProperty("password",  password);
        obj->setProperty("email", email);
        
        properties = var (obj);
        
        saveAllProperties();
    }
    
    void saveAllProperties()
    {
        String json = JSON::toString (properties);
        configFile.replaceWithText (json);
    }
    
    NamedValueSet getProperties()
    {
        return properties.getDynamicObject()->getProperties();
    }
    
private:
    File configFile;
    var properties;

};


#endif  // USERCONFIG_H_INCLUDED
