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
#include "UserConfig.h"

/** Class for accessing and querying the jpm Cloudant database
*/
class Database
{
public:

    Database(const String& username, const String& password) :
    request (Constants::databaseUrl),
    username (username),
    password (password)
    {
        request.header ("Content-Type", "application/json");
        request.header ("Authorization", "Basic " + getAuthToken(username, password));
    }

    Database() :
    request (Constants::databaseUrl),
    username (Constants::databaseKeyReadOnly),
    password (Constants::databasePasswordReadOnly)
    {
        request.header ("Content-Type", "application/json");
        request.header ("Authorization", "Basic " + getAuthToken(username, password));
    }

    /** Generate token for authorisation header
     */
    String getAuthToken(String username, String password) const
    {
        String s; 
        s += username;
        s += ":";
        s += password;

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

        adamski::RestRequest::Response response = request.get ("registry/_design/module-views/_view/all-modules").execute();

        DBG (response.bodyAsString);
        DBG (response.result.getErrorMessage());
        
        for (auto key : response.headers.getAllKeys())
        {
            DBG (key << ": " << response.headers.getValue(key, "n/a"));
        }

        if (checkStatus(response))
            return response.bodyAsString;
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
        // Cloudant's text query facility

        adamski::RestRequest::Response response = request.post ("registry/_find")
        .field ("selector", propertyAsVar("$text", searchString))
        .execute();

        checkStatus(response);

        DBG (response.bodyAsString);

        return response.bodyAsString;
    }

    /** Get all modules in a repository
     */
    String getModulesInRepo (const String& repoNameString)
    {

        // Cloudant's text query facility
        adamski::RestRequest::Response response = request.post ("registry/_find")
        .field ("selector", propertyAsVar("shortname", repoNameString))
        .execute();

        checkStatus(response);

        DBG (response.bodyAsString);

        return response.bodyAsString;
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

        adamski::RestRequest::Response response = request.get ("registry/_design/module-views/_view/module-name?key=%22" + moduleNameString + "%22").execute();

        checkStatus(response);

        return response.bodyAsString;
    }
    
    /** Add a user to the database. This is required in order to publish modules on the system.
     */
    String addUser (const String& username, const String& password, const String& email)
    {
        DBG ("username: " << this->username);
        DBG ("password: " << this->password);
        
        String salt = generateSalt();
        String id = "org.couchdb.user:" + username;
        
        adamski::RestRequest::Response response = request.put ("_users/" + id)
        .field ("type", "user")
        .field ("name", username)
        .field ("email", email)
        .field ("roles", Array<var>({var("publisher")}))
        .field ("password_sha", stringToSHA1 (password + salt))
        .field ("salt", salt)
        .execute();
        
        DBG (request.getBodyAsString());
        
        checkStatus(response, true);
        
        // If successful create/update config file: (create and overwrite for now)
        //

        return response.bodyAsString;
    }
    
    
    /**
     * TODO: Create 'CouchUser' class, so we can store current revision etc and pass in object to update/delete
     */
    String deleteUser (const String& username, const String& revision)
    {
        
    }
    
    String publish (var moduleInfo)
    {
        UserConfig userConfig;
        String id = moduleInfo["id"];
    
        // Set up maintainers object
        DynamicObject* obj = new DynamicObject;
        obj->setProperty("name", userConfig.getProperties()["username"]);
        obj->setProperty("email", userConfig.getProperties()["email"]);

        var maintainer = var(obj);
        Array<var> maintainers;
        maintainers.add (maintainer);
        
        // Get zipped source files
        MemoryOutputStream os;
        ZipFileUtilities::compressFolderToStream(File ("."), os);
        String zippedData = os.toString();

        // add as attachment to put request
        DynamicObject* attachmentData = new DynamicObject;
        attachmentData->setProperty ("content-type", "text/plain");
        attachmentData->setProperty( "data", Base64::toBase64 (zippedData));
        
        DynamicObject* attachmentObj = new DynamicObject;
        
        obj->setProperty(id + ".zip", var(attachmentData));
        var _attachments = var(attachmentObj);
        
        adamski::RestRequest::Response response = request.put ("registry/" + id)
        .field ("name", moduleInfo["name"])
        .field ("version", moduleInfo["version"])
        .field ("description", moduleInfo["description"])
        .field ("version", moduleInfo["version"])
        .field ("website", moduleInfo["website"])
        .field ("repository", moduleInfo["repository"])
        .field ("dependencies", moduleInfo["dependencies"])
        .field ("maintainers", maintainers)
        .field ("_attachments", _attachments)
        .execute();
        
        DBG (request.getBodyAsString());
        
        checkStatus(response);
        
        return response.bodyAsString;
    }

    const URL& getURL() const
    {
        return request.getURL();
    }
    
    
    
    
private:
    
    friend class Main__Tests; // This is just so we can call private functions from the tests.

    /** Generate salt for password. Returns a 32 character random hexadecimal string.
     */
    String generateSalt()
    {
        const char charset[] = "0123456789abcdef";
        Random random;
        String salt;
        for (int i = 0; i < 32; i++)
        {
            salt += charset[random.nextInt(16)];
        }
        
        DBG ("generated salt: " << salt);
        return salt;
        
    }
    
    /** Generate the SHA1 hash of a string 
     */
    String stringToSHA1 (String s)
    {
        unsigned char hash[20];
        char hexString[41];
        
        adamski::sha1::calc(s.toRawUTF8(), s.length(), hash);
        adamski::sha1::toHexString(hash, hexString);
        
        DBG ("generated password hash: " << hexString);
        return String(hexString);
    }
    
    /** Check for error status, print message and return false if error status found
     */
    bool checkStatus(adamski::RestRequest::Response res, bool fatal = false)
    {
        if (res.status < 200 || res.status > 300)
        {
            String errorString = "database returned status " + String(res.status);
            if (fatal)
            {
                JpmFatalExcepton (errorString, res.bodyAsString);
            }
            else
            {
                printError (errorString);
            printError(res.bodyAsString);
            }
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
    
    
    /** Function to simplify adding properties with / to vars
     */
    var propertyAsVar(const String& name, const String &value, var varToAddTo = var())
    {
        DynamicObject* obj = nullptr;
        if (varToAddTo.isVoid())
           obj  = new DynamicObject();
        else
           obj = varToAddTo.getDynamicObject();
            
        NamedValueSet& properties = obj->getProperties();
        properties.set (name, value);
        return var(obj);
    }

    adamski::RestRequest request;
    String username;
    String password;


};



#endif  // DATABASE_H_INCLUDED
