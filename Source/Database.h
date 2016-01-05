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
        request.header ("Authorization", "Basic " + getAuthToken(username, password));
        request.header ("Content-Type", "application/json");
    }

    Database() :
    request (Constants::databaseUrl),
    username (Constants::databaseKeyReadOnly),
    password (Constants::databasePasswordReadOnly)
    {
        request.header ("Authorization", "Basic " + getAuthToken(username, password));
        request.header ("Content-Type", "application/json");
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

    /** Get module matching a given id.
     */
    var getModuleById (const String& moduleId)
    {
        adamski::RestRequest::Response response = request.get ("registry/" + moduleId).execute();

        checkStatus(response);

        return response.body;
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

        return response.bodyAsString;
    }
    
    
    /**
     * TODO: Create 'CouchUser' class, so we can store current revision etc and pass in object to update/delete
     */
    String deleteUser (const String& username, const String& revision)
    {
        
    }
    
    /** Publish a module. Sends the contents of juce_module_info, plus other necessary data,
     * and uploads the zipped module folder. Assumes user is in the working directory of the module. 
     * Requires user config to have been created by `adduser` command.
     */
    String publish (var moduleInfo)
    {
        UserConfig userConfig;
        if (! userConfig.isValid())
        {
            JpmFatalExcepton ("Invalid or missing user config", "Please run adduser command");
        }
        
        String id = moduleInfo["id"];
    
        // Set up maintainers object
        DynamicObject* maintainerObj = new DynamicObject;
        maintainerObj->setProperty("name", userConfig.getProperties()["username"]);
        maintainerObj->setProperty("email", userConfig.getProperties()["email"]);

        var maintainer = var(maintainerObj);
        Array<var> maintainers;
        maintainers.add (maintainer);
        
        // Get zipped source files
        MemoryBlock memBlock;
        MemoryOutputStream os (memBlock, false);
        ZipFileUtilities::compressFolderToStream (File::getCurrentWorkingDirectory(), os);
    
//        // START DEBUG
//        File outputZip (File::getCurrentWorkingDirectory().getChildFile (id + ".zip"));
//        outputZip.replaceWithData(os.getData(), os.getDataSize());
//        
//        MemoryInputStream zipIS (os.getData(), os.getDataSize(), false);
//        ZipFile testZip (zipIS);
//        DBG ("Zipped data - number of entries: " << testZip.getNumEntries());
//        // END DEBUG
        
//        String encodedData = memBlock.toBase64Encoding();
//        DBG (encodedData);
        int compressedSize = os.getDataSize();
        DBG ("compressedSize: " << compressedSize);
        

//        // Add as attachment to put request -- this method does not work, unknown reason
//        DynamicObject* attachmentData = new DynamicObject;
//        attachmentData->setProperty ("content_type", "application/zip");
//        attachmentData->setProperty( "data", encodedData);
//        
//        DynamicObject* attachmentObj = new DynamicObject;
//        attachmentObj->setProperty(id + ".zip", var(attachmentData));
//        var attachments = var(attachmentObj);
        
        // First check if this document exists
        
        adamski::RestRequest::Response headResponse = request.head ("registry/" + id).execute();
        if (checkStatus (headResponse))
        {
            // Document exists, set revision from head response
            String rev = headResponse.headers.getValue("Etag", ""); // this works for HEAD request only
            request.field ("_rev", rev.removeCharacters("\""));
        }
        
        adamski::RestRequest::Response response = request.put ("registry/" + id)
        .field ("name", moduleInfo["name"])
        .field ("version", moduleInfo["version"])
        .field ("description", moduleInfo["description"])
        .field ("version", moduleInfo["version"])
        .field ("website", moduleInfo["website"])
        .field ("repository", moduleInfo["repository"])
        .field ("dependencies", moduleInfo["dependencies"])
        .field ("maintainers", maintainers)
        .field ("compressed-size", compressedSize)
        //.field ("_attachments", attachments)
        .execute();
        
        DBG (request.getBodyAsString());
        
        checkStatus(response, true);
        DBG (response.bodyAsString);
        
        // send attachment as PUT request
        adamski::RestRequest attachmentRequest(request.getURL());
        attachmentRequest.header ("Content-Type", "application/zip");
        attachmentRequest.header ("Content-Length", String (compressedSize));
        attachmentRequest.header ("Authorization", "Basic " + getAuthToken(username, password));
        DBG ("attachment size " + String (memBlock.getSize()));
        
        DBG (response.body["rev"].toString());
        
        adamski::RestRequest::Response attachmentResponse = request.put ("registry/" + id + "/" + id + ".zip?rev=" + response.body["rev"].toString())
        .data (memBlock)
        .execute();
        
        checkStatus(attachmentResponse, true);
        DBG (attachmentResponse.bodyAsString);
        
        return response.bodyAsString;
    }
    
    /** Get size of compressed attachment. Used by the install method to determine size of zip data */
    const int getAttachmentSize (const String& moduleId)
    {
        return getModuleById (moduleId)["compressed-size"].operator int();
    }
    
    /** Get the zipped source code */
    const MemoryBlock getZippedSource (const String& moduleId, var data = var::null)
    {
        int compressedSize;
        if (data.isVoid()) compressedSize = getAttachmentSize (moduleId);
        else compressedSize = data["compressed-size"];
        
        MemoryBlock memBlock (getAttachment(moduleId, moduleId+".zip"));
        memBlock.setSize (compressedSize); // necessary to be able to read the zip file (feels like a hack)
        return memBlock;
    }
    
    /** Get attachment from Cloudant db */
    const MemoryBlock getAttachment (const String& docId, const String& attachmentId)
    {
        request.removeHeader ("Content-Type");
        request.header ("Content-Type", "application/zip");
        adamski::RestRequest::Response attachmentResponse = request.get ("registry/" + docId + "/" + attachmentId)
        .execute();
        DBG ("request headers: " + request.getHeadersAsString());
        DBG ("request body: " + request.getBodyAsString());
        DBG ("bytes recieved: " + String (attachmentResponse.data.getSize()));
        DBG ("response headers: " + String (attachmentResponse.getHeadersAsString()));
        DBG ("response body: " + String (attachmentResponse.bodyAsString));
        
        return attachmentResponse.data;
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
    
    /** Check for error status, print message and return false if error status found.
     * If fatal is set to true it will fail with an exception on an unsuccesful request
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
