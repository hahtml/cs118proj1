#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "HttpHeader.h"

using namespace std;

class HttpRequest : public HttpHeader
{
 public:
  //default GET method
  HttpRequest(): m_method("GET") {}                      
  HttpRequest(string url): m_method("GET") {setUrl(url);} 
  //set methods:
  void setMethod(string method) {m_method = method;}
  void setUrl(string url);
  //get methods:
  string getMethod() const {return m_method;}
  string getPath() const {return m_path;}
  //encode request message into a wire
  wire_t encodeMsg(string entity);
  //parse encoded request message from a wire
  string consume(wire_t encodedMsg);

 private:
  string m_method;
  string m_path;
};

#endif
