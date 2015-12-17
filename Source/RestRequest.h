/*
  ==============================================================================

    RestRequest.h
    Created: 16 Dec 2015 2:29:23pm
    Author:  Adam Wilson

  ==============================================================================
*/

#ifndef RESTREQUEST_H_INCLUDED
#define RESTREQUEST_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class RestRequest
{
public:
    
    RestRequest (String urlString) : url (urlString) {}
    RestRequest (URL url)          : url (url) {}
    RestRequest () {}
    
    struct Response
    {
        Result result;
        StringPairArray headers;
        var body;
        int status;
        
        Response() : result (Result::ok()) {} // not sure about using Result if we have to initialise it to ok...
    } response;

    
    RestRequest::Response execute ()
    {
        auto urlRequest = url.getChildURL (endpoint);
        bool hasFields = (fields.getProperties().size() > 0);
        if (hasFields)
        {
            MemoryOutputStream output;
            fields.writeAsJSON(output, 2, false);
            urlRequest = urlRequest.withPOSTData (output.toString());
        }
        
        ScopedPointer<InputStream> in (urlRequest.createInputStream (hasFields, nullptr, nullptr, stringPairArrayToHeaderString(headers), 0, &response.headers, &response.status));
        
        response.result = checkInputStream (in);
        if (response.result.failed()) return response;
        
        //for (auto key : responseHeaders.getAllKeys())
        //{
        //    DBG (key << ": " << responseHeaders.getValue(key, "n/a"));
        //}
        //DBG (status);
        
        queryResult = in->readEntireStreamAsString();
        response.result = JSON::parse(queryResult, response.body);

        return response;
    }
 
    
    RestRequest get (const String& endpoint)
    {
        RestRequest req (*this);
        req.verb = "GET";
        req.endpoint = endpoint;
        
        return req;
    }

    RestRequest post (const String& endpoint)
    {
        RestRequest req (*this);
        req.verb = "POST";
        req.endpoint = endpoint;
        
        return req;
    }
    
    RestRequest put (const String& endpoint)
    {
        RestRequest req (*this);
        req.verb = "PUT";
        req.endpoint = endpoint;
        
        return req;
    }

    RestRequest del (const String& endpoint)
    {
        RestRequest req (*this);
        req.verb = "DELETE";
        req.endpoint = endpoint;
        
        return req;
    }
    
    RestRequest field (const String& name, const String& value)
    {
        RestRequest req (*this);
        fields.setProperty(name, var(value));
        return req;
    }
    
    RestRequest header (const String& name, const String& value)
    {
        RestRequest req (*this);
        headers.set (name, value);
        return req;
    }
    
    
private:
    URL url;
    StringPairArray headers;
    String queryResult;
    String verb;
    String endpoint;
    DynamicObject fields;
    
    Result checkInputStream (InputStream* in)
    {
        if (! in) return Result::fail ("RESTREQUEST request failed, check your internet connection");
        return Result::ok();
    }
    
    static String stringPairArrayToHeaderString(StringPairArray stringPairArray)
    {
        String result;
        for (auto key : stringPairArray.getAllKeys())
        {
            result += key + ": " + stringPairArray.getValue(key, "") + "\n";
        }
        return result;
    }
};



#endif  // RESTREQUEST_H_INCLUDED
