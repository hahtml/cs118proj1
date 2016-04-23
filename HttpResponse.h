#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include "HttpHeader.h"

using namespace std;

class HttpResponse : public HttpHeader
{
 public:
  //set methods:
  void setStatus(string status) {m_status = status;}
  void setStatusMsg(string statusMsg) {m_statusMsg = statusMsg;}
  //get methods:
  string getStatus() {return m_status;}
  string getStatusMsg() {return m_statusMsg;}
  //encode response message into a wire
  wire_t encodeMsg(string entity);
  //parse encoded response message from a wire
  string consume(wire_t encodedMsg);
  
 private:
  string m_status;
  string m_statusMsg;
};

#endif
