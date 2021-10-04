#pragma once

#include <string>

namespace vtb {

class BusTransaction : public VtbObject {

  using String = std::string;

public: 
  BusTransaction() {};
  ~BusTransaction() {};

  uint64_t timestamp;

protected: 
  virtual String getInfo() = 0;

private:

};

template <typename T_VtbAddress, typename T_VtbBus>
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
  AxiLiteTransaction( T_Type type, T_VtbAddress addr, T_VtbBus data, uint64_t mask = 0xFFFFFFFFFFFFFFFF, uint64_t time = 0) : addr(addr), data(data), type(type), mask(mask) { assert(sizeof(T_VtbBus) <=64); this->timestamp = time; };
  ~AxiLiteTransaction() {};
  
  String getInfo() override {return String(typeToString(type));}
  
  T_VtbAddress   addr;
  T_VtbBus   data;
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


template <typename T_VtbBus>
class AxiStreamTransaction : public BusTransaction {
  
  using String = std::string;

public: 
  
  enum T_Type{
    RANDOM, 
    ONE_PACKET,
    SINGLE_BEAT
  };
  
  AxiStreamTransaction() {memset(this, sizeof(this), 0);};      
  ~AxiStreamTransaction() {};
  
  String getInfo() override {return String(typeToString(type));}

  bool   getByte(uint64_t i, uint8_t* byte) const { *byte = (i >= offset && i<length)?data[i]:0x00; return (i >= offset && i < length);}
  
  T_Type   type;
  uint8_t *  data;
  uint64_t length;
  uint64_t offset;
  bool     last;
  uint64_t strb;
  uint64_t keep;

  T_VtbBus getBusData() {
    T_VtbBus data; 
    for(int i = 0; i < sizeof(T_VtbBus); i++)
    {
      vtb::setByte(&data, this->data[i], i);
    }
    return data;
  }

private:
  const char* typeToString(T_Type t) {
    switch(t) {
    case RANDOM         : return "RANDOM";
    case ONE_PACKET     : return "ONE_PACKET";
    case SINGLE_BEAT    : return "SINGLE_BEAT";
    default             : return "UNKNOWN";
    }
  }

};

template <typename T_VtbBus>
class AxiStreamPacket : public AxiStreamTransaction<T_VtbBus> {
  
  using String = std::string;

public: 
  
  AxiStreamPacket() = delete;
  AxiStreamPacket(const uint8_t* buffer, uint64_t length, uint64_t offset = 0, uint64_t time = 0) {
    
    //Allocate and align local buffer
    this->type = AxiStreamTransaction<T_VtbBus>::ONE_PACKET;
    this->offset = offset;
    this->data = new uint8_t[offset + length];
    memset(this->data, 0, offset);
    memcpy(this->data + offset, buffer, length);
    this->timestamp = time;
    this->length    = offset + length;
    }
  ~AxiStreamPacket() {};
  
private:
  
};

template <typename T_VtbBus>
class AxiStreamBeat : public AxiStreamTransaction<T_VtbBus> {

public:
  AxiStreamBeat (){};
  AxiStreamBeat (const T_VtbBus data, uint64_t keep, uint64_t strb, bool last, uint64_t time = 0) {
    this->type = AxiStreamTransaction<T_VtbBus>::SINGLE_BEAT;
    this->data = new uint8_t[sizeof(T_VtbBus)];
    for(int i = 0; i < sizeof(T_VtbBus); i++)
    {
      this->data[i] = vtb::getByte(data, i);
    }
    this->offset = 0;
    this->length = sizeof(T_VtbBus);
    this->keep = keep;;
    this->strb = strb;
    this->last = last;    
    this->timestamp = time;
  }

};

}
