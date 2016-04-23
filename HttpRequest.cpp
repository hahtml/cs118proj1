#include "HttpRequest.h"
#include <sstream>

using namespace std;

string HttpRequest::consume(wire_t encodedMsg)
{
  string request = decode(encodedMsg);
  istringstream buf(request);
  string line;
  getline(buf, line); //first line
  line = line.substr(0, line.length()-1); //get rid of carriage return
  size_t pos1, pos2, pos3;
  pos1 = line.find(" ") + 1;
  pos2 = line.find(" ", pos1);
  pos3 = line.find("HTTP/") + 5;
  int len1 = pos1-1, len2 = pos2-pos1, len3 = line.length()-pos3;
  m_method = line.substr(0, len1);
  m_path = line.substr(pos1, len2);
  setVersion(line.substr(pos3, len3));
  while (getline(buf, line)) {
    line = line.substr(0, line.length()-1); //get rid of carriage return
    if (!line.length())
      break; //ignore blank line
    pos1 = line.find(" ") + 1;
    len1 = pos1-2;
    len2 = line.length() - pos1;
    string field = line.substr(0, len1), description = line.substr(pos1, len2);
    setField(field, description);
  }
  pos1 = request.find("\r\n\r\n") + 4;
  string entity = request.substr(pos1, request.length()-pos1);
  return entity;
}

void HttpRequest::setUrl(string url)
{
  size_t pos1, pos2;
  pos1 = url.find("//") + 2;
  pos2 = url.find("/", pos1);
  int len1 = pos2-pos1, len2 = url.length()-pos2;
  m_path = url.substr(pos2, len2);
  string host = url.substr(pos1, len1);  
  setField("host", host);
}

wire_t HttpRequest::encodeMsg(string entity)
{
  string request = getMethod() + " " + getPath() + " HTTP/" + getVersion() + "\r\n";
  map<string,string> fields = getFields();
  map<string,string>::iterator iter;
  for(iter = fields.begin(); iter != fields.end(); iter++){
    request += iter->first + ": " + iter->second + "\r\n";
  }
  request += "\r\n";
  request += entity;
  return encode(request);
}
