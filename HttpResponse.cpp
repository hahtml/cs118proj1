#include "HttpResponse.h"
#include <iostream>
#include <sstream>


using namespace std;

wire_t HttpResponse::encodeMsg(string entity) {
  string response = "HTTP/" + getVersion() + " " + getStatus() + " " + getStatusMsg() + "\r\n";
  map<string,string> fields = getFields();
  map<string,string>::iterator iter;
  for(iter = fields.begin(); iter != fields.end(); iter++){
    response += iter->first + ": " + iter->second + "\r\n";
  }
  response += "\r\n";
  response += entity;
  return encode(response);
}

string HttpResponse::consume(wire_t encodedMsg) {
  string response = decode(encodedMsg);
  istringstream buf(response);
  string line;
  getline(buf, line); //first line
  line = line.substr(0, line.length()-1); //get rid of carriage return
  size_t pos1, pos2, pos3;
  pos1 = line.find("HTTP/") + 5;
  pos2 = line.find(" ") + 1;
  pos3 = line.find(" ", pos2) + 1;
  int len1 = pos2-pos1-1, len2 = pos3-pos2-1, len3 = line.length()-pos3;
  setVersion(line.substr(pos1, len1));
  setStatus(line.substr(pos2, len2));
  setStatusMsg(line.substr(pos3, len3));
  cout << line << endl; //print out the first header line
  while (getline(buf, line)) {
    line = line.substr(0, line.length()-1); //get rid of carriage return
    if (!line.length())
      break; //ignore blank line
    pos1 = line.find(" ") + 1;
    len1 = pos1-2;
    len2 = line.length() - pos1;
    string field = line.substr(0, len1), description = line.substr(pos1, len2);
    setField(field, description);
    cout << line << endl; //print out additional header line
  }
  pos1 = response.find("\r\n\r\n") + 4;
  string entity = response.substr(pos1, response.length()-pos1);
  return entity;
}
