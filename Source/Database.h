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

#define DATABASE_URL        "https://codegarden.cloudant.com/jpm/"
#define READ_ONLY_KEY       "romenglentiouldissionged"
#define READ_ONLY_PASSWORD  "cbe04fcf61bd85eb946e31ce0c310adabc2986b4"

/** Class for accessing and querying the jpm Cloudant database
*/
class Database
{
public:
    
    Database()
    :   url (DATABASE_URL)
    {
        headers = String("Content-Type:application/json\n" \
                         "Authorization: Basic " + Base64::toBase64 (String(READ_ONLY_KEY) + ":" + String(READ_ONLY_PASSWORD)));
    }
    
    
    /** Massage the returned data into a simpler array format
     */
    static var parseToArray(String json)
    {
        var array;
        var parsedJson = JSON::parse(json);
        var rows = parsedJson["rows"];
        for (auto row : *rows.getArray())
        {
            array.append(row["value"]);
        }
        return array;
    }
    
    
    /** Get all repositories and module data as text for local storage
     */
    String getAllModulesAsJSON()
    {
        // Call predefined "all-modules" view.
        
        get ("_design/module-views/_view/all-modules");
        
        DBG (queryResult);
        
        if (checkStatus())
            return queryResult;
        else
            return String();
    }
    
//    /** Convert all-modules view result into ValueTree
//     */
//    ValueTree modulesJSONToDirectory (String modulesJSON)
//    {
//        var directory;
//        var parsedQuery = JSON::parse (modulesJSON);
//        // Get our data from return rows
//        var rows = parsedQuery["rows"];
//        if (rows.isArray())
//        {
//            for (auto row : *rows.getArray())
//            {
//                var value = row["value"];
//                directory.append (value);
//            }
//        }
//    }

    String textSearch (const String & searchString)
    {
        String query = String ("{ \"selector\": { \"$text\": \"" + searchString + "\" } }");
        //DBG (query);
        
        // Cloudant's text query facility
        post ("/_find", query);
        
        checkStatus();
        

        DBG (queryResult);
        
        /*
        var parsedQuery = JSON::parse (queryResult);

        // Traverse down the var to grab our data
        var docs = parsedQuery["docs"];
        if (docs.isArray())
        {
            for (auto repo : *docs.getArray())
            {
                var moduleSet = repo["module"];
                if (moduleSet.isArray())
                {
                    for (auto module : *moduleSet.getArray())
                    {
                        DBG (module["name"].toString());
                        result.add (createModuleFromVar (repo, module));
                    }
                }
                else
                {
                    DBG (moduleSet.getProperty("name", "n/a").toString());
                    result.add (createModuleFromVar (repo, moduleSet));
                }
            }
        }
         */
        
        return queryResult;
    }
    
    String getModulesInRepo (const String & repoNameString)
    {
        
        // Build query string to find module or module set based on shortname
        String query = String ("{ \"selector\": { \"shortname\": \"" + repoNameString + "\" } }");
        //DBG (query);
        
        // Cloudant's text query facility
        post ("/_find", query);
        
        checkStatus();
        
        DBG (queryResult);
        
        /*
        var parsedQuery = JSON::parse (queryResult);
        
        // Traverse down the var to grab our data
        var docs = parsedQuery["docs"];
        if (docs.isArray())
        {
            for (auto repo : *docs.getArray())
            {
                var moduleSet = repo["module"];
                if (moduleSet.isArray())
                {
                    for (auto module : *moduleSet.getArray())
                    {
                        DBG (module["name"].toString());
                        result.add (createModuleFromVar (repo, module));
                    }
                }
                else
                {
                    DBG (moduleSet.getProperty("name", "n/a").toString());
                    result.add (createModuleFromVar (repo, moduleSet));
                }
            }
        }
        */

        return queryResult;
    }
    
    /** Get modules matching a given name.
     *  This calls a pre-defined 'view' which is a javascript map function defined on the database.
     *
     *  For reference the map function, as of the last update to this file:
     *      function (doc) {
     *          if (doc.module) {
     *              if (Array.isArray(doc.module)) {
     *                  doc.module.forEach (function (module) {
     *                      emit (module.name, {repo: {shortname: doc.shortname, path: doc.path, source: doc.source}, module: module});
     *                  });
     *              } else {
     *                  emit (doc.module.name, {repo: {shortname: doc.shortname, path: doc.path, source: doc.source}, module: doc.module});
     *              }
     *          }
     *      }
     */
    String getModulesByName (const String & moduleNameString)
    {
//        Array<Module> result;
//        ModuleName module(moduleNameString);
        
        // Call predefined "module-name" view.
        
        get ("_design/module-views/_view/module-name?key=%22" + moduleNameString + "%22");
        
        checkStatus();
        
//        DBG (queryResult);
//        var parsedQuery = JSON::parse (queryResult);
//        
//        // Get our data from return rows
//        var rows = parsedQuery["rows"];
//        if (rows.isArray())
//        {
//            for (auto row : *rows.getArray())
//            {
//                var value = row["value"];
//                result.add (createModuleFromVar (value["repo"], value["module"]));
//            }
//        }

        return queryResult;
    }
    
    URL &getURL()
    {
        return url;
    }

private:
    
    /** Check for error status, print message and return false if error status found
     */
    bool checkStatus()
    {
        if (status < 200 || status > 300)
        {
            std::cerr << "fatal: returned status " << status << ":" << std::endl;
            std::cerr << queryResult;
            return false;
        }
        return true;
    }

    
//    /**
//     Create and return a Module given a var that represents a returned repository,
//     and a var that represents a single module entry
//     */
//    Module createModuleFromVar (var repo, var module)
//    {
//        auto test = [&repo](const String & text)
//        {
//            if (text.isEmpty())
//                std::cerr << "warning: error in json for repo " << repo["shortname"].toString() << std::endl;
//            return text;
//        };
//        
//        Module m;
//        
//        m.setRepo (test (repo["shortname"]));
//        m.setPath (test (repo["path"]));
//        m.setSource (test (repo["source"]));
//        m.setName (test (module["name"]));
//        m.setSubPath (test (module["subpath"]));
//        
//        /* Description is allowed to be empty. */
//        m.setDescription (module["description"]);
//        
//        return m;
//    }
    
    /**
     create a GET HTTP request to an endpoint based on the stored URL
     and store the result
     */
    void get (const String & endpoint)
    {
        const ScopedPointer<InputStream> in(url.getChildURL (endpoint).createInputStream (false, nullptr, nullptr, headers, 0, &responseHeaders, &status));
        
        //for (auto key : responseHeaders.getAllKeys())
        //{
        //    DBG (key << ": " << responseHeaders.getValue(key, "n/a"));
        //}
        //DBG (status);

        queryResult = in->readEntireStreamAsString();
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
