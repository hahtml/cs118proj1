#include "HttpHeader.h"
#include <utility>

using namespace std;

void insertIntoWire(wire_t* wire, char* buf, ssize_t len) {
  wire->insert(wire->end(), buf, buf+len);
}

void HttpHeader::setField(string field, string description)
{
  m_fields.insert(pair<string,string>(field, description));
}

wire_t HttpHeader::encode(string originMsg)
{
    wire_t encodedMsg;
    for (size_t i = 0; i < originMsg.length(); i++) {
      encodedMsg.push_back((uint8_t)originMsg[i]);
    }
    return encodedMsg;
}

string HttpHeader::decode(wire_t encodedMsg)
{
  string request = "";
  for(size_t i = 0; i< encodedMsg.size(); i++){
    request += (char)encodedMsg[i];
  }
  return request;
}
