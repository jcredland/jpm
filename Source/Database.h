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
#include "Constants.h"

/** Class for accessing and querying the jpm Cloudant database
*/
class Database
{
public:

    Database() : url (Constants::databaseUrl)
    {
        headers = String ("Content-Type:application/json\n"
                          "Authorization: Basic " + getAuthToken());
    }

    /** Generate token for authorisation header
     */
    String getAuthToken() const 
    {
        String s; 
        s += Constants::databaseKeyReadOnly; 
        s += ":";
        s += Constants::databasePasswordReadOnly; 

        return Base64::toBase64 (s); 
    }


    /** Massage the returned data into a simpler array format
     */
    static var parseToArray (String json)
    {
        var array;
        var parsedJson = JSON::parse (json);
        var rows = parsedJson["rows"];
        
        if (rows.isVoid())
        {
            throw JpmFatalExcepton ("no rows found in data",
                                    "Check cached data file for debugging which should contain any returned data");
        }
        
        for (auto row : *rows.getArray())
        {
            var value = row["value"];
            var key = row["key"];
            if (value != var::null)
            {
                array.append(value);
            }
            else
            {
                if (key.isVoid())
                    printError ("no value or key found for row");
                else
                    printError ("no value found for key " + row["key"].toString());
            }
        }

        return array;
    }


    /** Get all repositories and module data as text for local storage
     */
    String getAllModules()
    {
        // Call predefined "all-modules" view.

        get ("_design/module-views/_view/all-modules");

        DBG (queryResult);

        if (checkStatus())
            return queryResult;
        else
            return String();
    }

    /** Full text search using Cloudant's Lucene search facility
     *
     *  This needs some work to make it useful. Cloudant does not allow wildcards at the
     *  start of a search string (as it can be resource hungry) so it can't currently
     *  be used for the 'list' functionality.
     */
    String textSearch (const String& searchString)
    {
        String query = String ("{ \"selector\": { \"$text\": \"" + searchString + "\" } }");
        //DBG (query);

        // Cloudant's text query facility
        post ("/_find", query);

        checkStatus();

        DBG (queryResult);

        return queryResult;
    }

    /** Get all modules in a repository
     */
    String getModulesInRepo (const String& repoNameString)
    {

        // Build query string to find module or module set based on shortname
        String query = String ("{ \"selector\": { \"shortname\": \"" + repoNameString + "\" } }");
        //DBG (query);

        // Cloudant's text query facility
        post ("/_find", query);

        checkStatus();

        DBG (queryResult);

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
    String getModulesByName (const String& moduleNameString)
    {
        // Call predefined "module-name" view.

        get ("_design/module-views/_view/module-name?key=%22" + moduleNameString + "%22");

        checkStatus();

        return queryResult;
    }

    const URL& getURL() const
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
            printError("fatal: database returned status " + String(status) + ":");
            printError(queryResult);
            return false;
        }

        return true;
    }
    
    /** Generate exception if InputStream is null
     */
    void checkInputStream (InputStream* in, URL urlRequest)
    {
        if (! in)
            throw JpmFatalExcepton ("could not create input stream for: " + urlRequest.toString (true),
                                    "probably a build problem on linux where https support hasn't been compiled in");
    }

    /**
     create a GET HTTP request to an endpoint based on the stored URL
     and store the result
     */
    void get (const String& endpoint)
    {
        auto urlRequest = url.getChildURL (endpoint);

        ScopedPointer<InputStream> in (urlRequest.createInputStream (false, nullptr, nullptr, headers, 0, &responseHeaders, &status));

        checkInputStream (in, urlRequest);

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
    void post (const String& endpoint, const String& request)
    {
        auto urlRequest = url.getChildURL (endpoint).withPOSTData (request);

        ScopedPointer<InputStream> in (urlRequest.createInputStream (true, nullptr, nullptr, headers, 0, &responseHeaders, &status));

        checkInputStream (in, urlRequest);

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
