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

inline void printHeading (const String& s)
{
    std::cout << "jpm ****** " << s << std::endl;
}

inline void printWarning (const String& s)
{
    std::cout << "jpm -    : " << s << std::endl;
}

inline void printInfo (const String& s)
{
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

#endif  // UTILITIES_H_INCLUDED
