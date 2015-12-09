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
        DBG (query);
        // Cloudant's text query facility
        post ("/_find", query);
        
        var parsedQuery = JSON::parse (queryResult);
        
        var docs = parsedQuery.getProperty("docs", "not_found");
        if (docs.isArray())
        {
            
        }
        // use isArray etc to check parsedQuery...
//        for(auto item : *parsedQuery.getProperty("docs", "").getArray()->getFirst().getProperty("module", "").getArray())
//        {
//            DBG (item.getProperty("name", "n/a").operator String());
//        }
        return result;
    }
    
private:
    
    void post (const String & endpoint, const String & request)
    {
        const ScopedPointer<InputStream> in(url.getChildURL (endpoint).withPOSTData (request).createInputStream (true, nullptr, nullptr, headers, 0, &responseHeaders, &status));
        for (auto key : responseHeaders.getAllKeys())
        {
            DBG (key << ": " << responseHeaders.getValue(key, "n/a"));
        }
        DBG (status);
        DBG (in->readEntireStreamAsString());
        queryResult = in->readEntireStreamAsString();
    }
    
    URL url;
    StringPairArray responseHeaders;
    String headers;
    String queryResult;
    int status;
    
};



#endif  // DATABASE_H_INCLUDED
