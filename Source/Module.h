/*
  ==============================================================================

    Module.h
    Created: 7 Dec 2015 1:32:17pm
    Author:  Jim

  ==============================================================================
*/

#ifndef MODULE_H_INCLUDED
#define MODULE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "DownloadCache.h"
#include "Source_GitHub.h"

/** Decodes module names in the format repo/name@version */
class ModuleName
{
public:
    ModuleName (const String& name_) : name (name_) {}

    String getRepo() const
    {
        return name.contains ("/") ? name.upToFirstOccurrenceOf ("/", false, false) : String::empty;
    }

    String getVersion() const
    {
        return name.contains ("@") ? name.fromLastOccurrenceOf ("@", false, false) : String::empty;
    }

    String getName() const
    {
        if (name.contains ("/"))
            return name.fromFirstOccurrenceOf ("/", false, false).upToLastOccurrenceOf ("@", false, false);

        return name.upToLastOccurrenceOf ("@", false, false);
    }
private:
    String name;
};

/** Refers to a module. */
class Module
{
public:
    Module() : state ("module") {}

    Module (ValueTree& state_)
    {
        state = state_;

        if (! getValidSources().contains (getSource()))
            std::cerr << "error: invalid source: " << getSource() << std::endl;
    }

    ValueTree getStateAsValueTree() const
    {
        return state;
    }

    bool isValid() const
    {
        return getName().isNotEmpty();
    }

    /** Return a one line view that'll be used for 'jpm list' */
    String getSummaryString() const
    {
        String name = getRepo() + "/" + getName();
        return name.paddedRight (' ', 40) + getDescription();
    }

    /** Install this module into a destination folder. */
    void install (const File& destinationFolder)
    {
        File file;

        if (getSource() == "GitHub")
        {
            GitHubSource github;
            auto result = github.download (getPath(), getVersion(), getSubPath());

            if (! result.success)
                return;

            file = result.file;
            setVersion (result.actualVersionNumber);
        }
        else if (getSource() == "LocalPath")
        {

        }
        else
        {
            std::cerr << "Invalid source " + getSource() << std::endl;
        }

        if (! file.exists())
        {
            std::cerr << "Invalid module" << std::endl;
            return;
        }

        auto result = file.copyDirectoryTo (destinationFolder.getChildFile (getName()));

        if (!result)
        {
            std::cerr << "Problem copying module" << std::endl;
        }
    }

    /* Getters and setters. */
    String getName() const
    {
        return state["name"];
    }
    void setName (const String& name)
    {
        state.setProperty ("name", name, nullptr);
    }
    String getDescription() const
    {
        return state["description"];
    }
    void setDescription (const String& description)
    {
        state.setProperty ("description", description, nullptr);
    }
    String getVersion() const
    {
        return state["version"];
    }
    void setVersion (const String& version)
    {
        state.setProperty ("version", version, nullptr);
    }
    String getSource() const
    {
        return state["source"];
    }
    void setSource (const String& source)
    {
        state.setProperty ("source", source, nullptr);
    }
    String getPath() const
    {
        return state["path"];
    }
    void setPath (const String& path)
    {
        state.setProperty ("path", path, nullptr);
    }
    String getSubPath() const
    {
        return state["subpath"];
    }
    void setSubPath (const String& subpath)
    {
        state.setProperty ("subpath", subpath, nullptr);
    }
    String getRepo() const
    {
        return state["repo"];
    }
    void setRepo (const String& repo)
    {
        state.setProperty ("repo", repo, nullptr);
    }
private:
    static StringArray& getValidSources()
    {
        static StringArray validSources;
        validSources.add ("GitHub");
        validSources.add ("LocalPath");
        return validSources;
    }

    ValueTree state;
};



#endif  // MODULE_H_INCLUDED
