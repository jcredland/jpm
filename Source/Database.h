/*
  ==============================================================================

    Database.h
    Created: 8 Dec 2015 11:04:49pm
    Author:  Adam Wilson

  ==============================================================================
*/

#ifndef DATABASE_H_INCLUDED
#define DATABASE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Module.h"
#include "DownloadCache.h"
#include <iostream>

#define DATABASE_URL        "https://codegarden.cloudant.com/jpm/"
#define READ_ONLY_KEY       "romenglentiouldissionged"
#define READ_ONLY_PASSWORD  "cbe04fcf61bd85eb946e31ce0c310adabc2986b4"

class Database 
{
public:
    
    Database()
    :   url (DATABASE_URL)
    {
        headers = String("Content-Type:application/json\n" \
                         "Authorization: Basic " + Base64::toBase64 (String(READ_ONLY_KEY) + ":" + String(READ_ONLY_PASSWORD)));
    }
    
    Array<Module> getModulesByName (const String & moduleNameString)
    {
        Array<Module> result;
        
        // Build query string to find module or module set based on shortname
        String query = String ("{ \"selector\": { \"shortname\": \"" + moduleNameString + "\" } }");
        //DBG (query);
        
        // Cloudant's text query facility
        post ("/_find", query);
        
        if (status < 200 || status > 300)
        {
            std::cerr << "fatal: returned status " << status << ":" << std::endl;
            std::cerr << queryResult;
        }
        
        //DBG (queryResult);
        var parsedQuery = JSON::parse (queryResult);
        
        // Traverse down the var to grab our data
        var docs = parsedQuery["docs"];
        if (docs.isArray())
        {
            for (auto doc : *docs.getArray())
            {
                var moduleSet = doc["module"];
                if (moduleSet.isArray())
                {
                    for (auto module : *moduleSet.getArray())
                    {
                        DBG (module["name"].toString());
                        result.add (createModuleFromVar (doc, module));
                    }
                }
                else
                {
                    DBG (moduleSet.getProperty("name", "n/a").toString());
                    result.add (createModuleFromVar (doc, moduleSet));
                }
            }
        }

        return result;
    }
    
private:
    
    /**
     Create and return a Module given a var that represents a returned document,
     and a var that represents a single module entry
     */
    Module createModuleFromVar (var doc, var module)
    {
        auto test = [&doc](const String & text)
        {
            if (text.isEmpty())
                std::cerr << "warning: error in json for repo " << doc["shortname"].toString() << std::endl;
            return text;
        };
        
        Module m;
        
        m.setRepo (test (doc["shortname"]));
        m.setPath (test (doc["path"]));
        m.setSource (test (doc["source"]));
        m.setName (test (module["name"]));
        m.setSubPath (test (module["subpath"]));
        
        /* Description is allowed to be empty. */
        m.setDescription (module["description"]);
        
        return m;
    }
    
    /**
     create a POST HTTP request to an endpoint based on the stored URL
     and store the result
     */
    void post (const String & endpoint, const String & request)
    {
        const ScopedPointer<InputStream> in(url.getChildURL (endpoint).withPOSTData (request).createInputStream (true, nullptr, nullptr, headers, 0, &responseHeaders, &status));
        
        //for (auto key : responseHeaders.getAllKeys())
        //{
        //    DBG (key << ": " << responseHeaders.getValue(key, "n/a"));
        //}
        //DBG (status);

        queryResult = in->readEntireStreamAsString();
    }
    
    URL url;
    StringPairArray responseHeaders;
    String headers;
    String queryResult;
    int status;
    
};



#endif  // DATABASE_H_INCLUDED
