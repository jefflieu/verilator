#pragma once

#include <string>

namespace vtb {

class BusTransaction {

  using String = std::string;

public: 
  BusTransaction() {};
  ~BusTransaction() {};

  uint64_t timestamp;

protected: 
  virtual String getInfo() = 0;

private:

};

template <typename Type_A, typename Type_D>
class AxiLiteTransaction : public BusTransaction {
  
  using String = std::string;

public: 
  
  enum T_Type{
    WRITE, 
    READ,
    READ_CHECK,
    READ_POLL,
    READ_MOD_WRITE    
  };
  
  AxiLiteTransaction() {};
  AxiLiteTransaction( T_Type type, Type_A addr, Type_D data, uint64_t mask = 0xFFFFFFFFFFFFFFFF, uint64_t time = 0) : addr(addr), data(data), type(type), mask(mask) { assert(sizeof(Type_D) <=64); this->timestamp = time; };
  ~AxiLiteTransaction() {};
  
  String getInfo() override {return String(typeToString(type));}
  
  Type_A   addr;
  Type_D   data;
  uint64_t mask;
  T_Type   type;

private:
  const char* typeToString(T_Type t) {
    switch(t) {
    case WRITE          : return "WRITE";
    case READ           : return "READ";
    case READ_CHECK     : return "READ_CHECK";
    case READ_POLL      : return "READ_POLL";
    case READ_MOD_WRITE : return "WRITE";
    default             : return "UNKNOWN";
    }
  }

};


class AxiStreamTransaction : public BusTransaction {
  
  using String = std::string;

public: 
  
  enum T_Type{
    RANDOM, 
    FROM_BUFFER
  };
  
  AxiStreamTransaction() {};
  AxiStreamTransaction( T_Type type, uint8_t* buffer, uint64_t length, uint64_t offset = 0, uint64_t time = 0) : type(type), offset(offset) {
    //Allocate and align local buffer   
    this->data = new uint8_t[offset + length];
    memset(this->data, 0, offset);
    memcpy(this->data + offset, buffer, length);
    this->timestamp = time;
    this->length    = offset + length;
    }
  ~AxiStreamTransaction() {};
  
  String getInfo() override {return String(typeToString(type));}
  bool   getByte(uint64_t i, uint8_t* byte) { *byte = (i >= offset && i<length)?data[i]:0x00; return (i >= offset && i < length);}
  uint8_t *  data;
  uint64_t length;
  uint64_t offset;
  T_Type   type;

private:
  const char* typeToString(T_Type t) {
    switch(t) {
    case RANDOM         : return "RANDOM";
    case FROM_BUFFER    : return "FROM_BUFFER";
    default             : return "UNKNOWN";
    }
  }

};


}
