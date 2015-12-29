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

/**
 * OutputRecorder buffers anything printed to the screen.  It makes
 * automated tests easier to write. 
 */
class OutputRecorder
{
public:
    void record(const String & s)
    {
        lines.add(s); 
    }

    /** Returns true if the output contained the exact string specified. */
    bool hasLineMatching(const String & s) const
    {
        return lines.contains(s);
    }

    /** Returns true if the output contained the substring specified. */
    bool hasLineContaining(const String & s) const
    {
        DBG("** " << lines.size());

        for (auto l: lines)
            if (l.contains(s))
                return true;

        return false;
    }

private:
    StringArray lines;
};

extern ScopedPointer<OutputRecorder>  outputRecorder;

inline void printHeading (const String& s)
{
    if (outputRecorder)
        outputRecorder->record(s); 

    std::cout << "jpm ****** " << s << std::endl;
}

inline void printWarning (const String& s)
{
    if (outputRecorder)
        outputRecorder->record(s); 

    std::cout << "jpm -    : " << s << std::endl;
}

inline void printInfo (const String& s)
{
    if (outputRecorder)
        outputRecorder->record(s); 

    std::cout << "jpm      : " << s << std::endl;
}

inline void printError (const String& s)
{
    std::cout << "jpm error: " << s << std::endl;
}


/** Provides an STL compatible iterator for the children of ValueTree. */
class ValueTreeChildrenConnector
{
public:
    ValueTreeChildrenConnector (const ValueTree& tree) : tree (tree) {}

    class Iterator
    {
    public:
        Iterator (ValueTree& tree, int position) : tree (tree), pos (position) {}
        Iterator& operator++()
        {
            ++pos;
            return *this;
        }
        bool operator!= (const Iterator& other) const
        {
            return other.pos != pos || other.tree != tree;
        }
        ValueTree operator * () const
        {
            return tree.getChild (pos);
        }

    private:
        Iterator& operator= (const Iterator&) = delete;
        ValueTree& tree;
        int pos;
    };

    Iterator begin()
    {
        return Iterator (tree, 0);
    }
    Iterator end()
    {
        return Iterator (tree, tree.getNumChildren());
    }

private:
    JUCE_DECLARE_NON_COPYABLE (ValueTreeChildrenConnector)
    ValueTree tree;
};

class ZipFileUtilities
{
public:
    /** Compress file folder and save as a zip file.
     */
    static void compressFolderToStream (const File& path, OutputStream& outputStream)
    {
        ZipFile::Builder zipBuilder;
        Array<File> tempFiles;
        path.findChildFiles(tempFiles, File::findFiles, true, "*");
        
        //add files
        for (int i = 0; i < tempFiles.size(); i++)
        {
            zipBuilder.addFile(tempFiles[i], 9, tempFiles[i].getRelativePathFrom(path));
        }
        
        double *progress = nullptr;
        zipBuilder.writeToStream(outputStream, progress);
    }
    
    static void compressFolder (const File& path, File outputZip)
    {

        
        //save our zip file

        if (outputZip.exists())
        {
            outputZip.deleteFile();
        }
        FileOutputStream os (outputZip);

        compressFolderToStream (path, os);
    }
};

#endif  // UTILITIES_H_INCLUDED
