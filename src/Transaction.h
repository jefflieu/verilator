




#pragma once


class BusTransaction {

public: 
  BusTransaction() {};
  ~BusTransaction() {};
  
private:

};

class AxiLiteTransaction : public BusTransaction {
public: 
  
  enum T_Type{
    WRITE, 
    READ,
    READ_CHECK,
    POLL_CHECK,
    READ_MOD_WRITE,
  };
  
  AxiLiteTransaction() {};
  AxiLiteTransaction( T_Type type, uint64_t addr, uint64_t data, uint64_t mask = 0xFFFFFFFFFFFFFFFF, uint64_t time = 0) : timestamp(time), addr(addr), data(data), type(type), mask(mask) {};
  ~AxiLiteTransaction() {};
  

  uint64_t timestamp;
  uint64_t addr;
  uint64_t data;
  uint64_t mask;
  T_Type   type;

private:

};