/*
  ==============================================================================

    ConfigFile.h
    Created: 7 Dec 2015 4:18:02pm
    Author:  Jim

  ==============================================================================
*/

#ifndef CONFIGFILE_H_INCLUDED
#define CONFIGFILE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Module.h"

/** Holds the saved JPM project configuration.  Contains module sources, version numbers and so on. */
class ConfigFile
{
public:
    ConfigFile (const File& file_)
        :
        file (file_),
        config ("jpm_config")
    {
        ScopedPointer<XmlElement> xml = XmlDocument (file).getDocumentElement();

        if (!xml)
            return; /* Not unexpected - there might be no initial configuration. */

        config = ValueTree::fromXml (*xml);
    }

    ~ConfigFile()
    {
        /* This operation always feels risky ... should we do some backup and validation? */
        file.replaceWithText (config.toXmlString());
    }

    /** Returns the array of Module objects from the configuration. */
    Array<Module> getModules()
    {
        Array<Module> result;

        for (int i = 0; i < config.getNumChildren(); ++i)
        {
            auto data = config.getChild (i);
            result.add (Module (data));
        }

        return result;
    }

    /** Adds a new module object to the configuration. */
    void addModule (Module m)
    {
        auto child = m.getStateAsValueTree();

        auto existingEntry = config.getChildWithProperty ("name", m.getName());

        if (existingEntry != ValueTree::invalid)
            config.removeChild (existingEntry, nullptr);

        config.addChild (child, -1, nullptr);
    }


    /** Returns the config as a string - this is for debugging. */
    String toString() const
    {
        return config.toXmlString();
    }

private:
    File file;
    ValueTree config;
};



#endif  // CONFIGFILE_H_INCLUDED
