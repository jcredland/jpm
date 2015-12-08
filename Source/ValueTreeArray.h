/*
  ==============================================================================

    ValueTreeArray.h
    Created: 7 Dec 2015 1:25:50pm
    Author:  Jim

  ==============================================================================
*/

#ifndef VALUETREEARRAY_H_INCLUDED
#define VALUETREEARRAY_H_INCLUDED


template <class ItemType>
class ValueTreeArray
{
public:

    class Iterator
    {
    public:
        Iterator (const ValueTreeArray* ptr, int pos)
            : pos (pos), ptr (ptr)
        { }
        bool operator!= (const Iterator& other) const
        {
            return pos != other.pos;
        }
        ItemType operator* () const
        {
            return ptr->get (pos);
        };
        const Iterator& operator++ ()
        {
            ++pos;
            return *this;
        }
    private:
        int pos;
        const ValueTreeArray* ptr;
    };


    ValueTreeArray (const String& nodeTypeName, const ValueTree& treeForData)
        :
        nodeTypeName (nodeTypeName),
        tree (treeForData)
    {
        valueTreeAction.setValueTree (tree);
        valueTreeAction.setAction (
            [this]()
        {
            listeners.call (&Listener::arrayDataUpdated);
        });

        jassert (tree != ValueTree::invalid);
    }

    ~ValueTreeArray() {}

    ItemType operator[] (int index)
    {
        return ItemType (tree.getChild (index));
    }
    ItemType get (int index) const
    {
        return ItemType (tree.getChild (index));
    }

    Iterator begin() const
    {
        return Iterator (this, 0);
    }
    Iterator end() const
    {
        return Iterator (this, size());
    }
    int size() const
    {
        return tree.getNumChildren();
    }

    ItemType getNewNode()
    {
        ValueTree itemTree (nodeTypeName);
        tree.addChild (itemTree, -1, nullptr);
        ItemType newItem (itemTree);
        return newItem;
    }

    /** Inserts something after the given location. */
    ItemType insertAfter (int location)
    {
        ValueTree itemTree (nodeTypeName);
        tree.addChild (itemTree, location + 1, nullptr);
        ItemType newItem (itemTree);
        return newItem;
    }


    /** Calls getTree() on the ItemType and adds it to the
    array. */
    void add (ItemType item)
    {
        tree.addChild (item.getTree(), -1, nullptr);
    }

    void remove (int index)
    {
        tree.removeChild (index, nullptr);
    }
    void clear()
    {
        tree.removeAllChildren (nullptr);
    }

    bool operator== (const ValueTreeArray& rhs) const
    {
        return tree.isEquivalentTo (rhs.tree);
    }

    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void arrayDataUpdated() = 0;
    };

    void addListener (Listener* listener)
    {
        listeners.add (listener);
    }

private:
    ListenerList<Listener> listeners;
    ValueTreeActionOnAnyChange valueTreeAction;
    String nodeTypeName;
    ValueTree tree;
    JUCE_LEAK_DETECTOR (ValueTreeArray)
};




#endif  // VALUETREEARRAY_H_INCLUDED
