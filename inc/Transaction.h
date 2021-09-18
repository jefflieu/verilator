#pragma once

#include <string>

namespace vtb {

class BusTransaction {

  using String = std::string;

public: 
  BusTransaction() {};
  ~BusTransaction() {};
  virtual String getInfo() = 0;
private:

};

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
  AxiLiteTransaction( T_Type type, uint64_t addr, uint64_t data, uint64_t mask = 0xFFFFFFFFFFFFFFFF, uint64_t time = 0) : timestamp(time), addr(addr), data(data), type(type), mask(mask) {};
  ~AxiLiteTransaction() {};
  
  String getInfo() override {return String(typeToString(type));}

  uint64_t timestamp;
  uint64_t addr;
  uint64_t data;
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

}
