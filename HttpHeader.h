#ifndef HTTPHEADER_H
#define HTTPHEADER_H

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

typedef vector<uint8_t> wire_t;

#define MAXSIZE 1024

void insertIntoWire(wire_t* wire, char* buf, ssize_t len);

class HttpHeader
{
 public:  
  HttpHeader(): m_version("1.0") {}; //default HTTP version
  //set methods:
  void setVersion(string version) {m_version = version;}
  void setField(string field, string description);
  //get methods:
  string getVersion() const {return m_version;}
  string getField(string field) {return m_fields.find(field)->second;}
  map<string,string> getFields() const {return m_fields;}
  //encode & decode:
  wire_t encode(string originMsg);
  string decode(wire_t encodedMsg);

 private:
  string m_version;
  map<string,string> m_fields;
};

#endif


