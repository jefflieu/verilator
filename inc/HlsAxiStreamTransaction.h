
#pragma once

#include <string>

#include "hls_stream.h"

namespace vtb {


template <class HlsAxiStream>
class HlsAxiStreamTransaction : public AxiStreamTransaction, public HlsAxiStream {
  
  using String = std::string;
  
public: 
  
  HlsAxiStreamTransaction() {};  
  ~HlsAxiStreamTransaction() {};



protected:
  HlsStream stream;

  
  String getInfo() override {return String(typeToString(type));}
  bool   getByte(uint64_t i, uint8_t* byte) { *byte = (i >= offset && i<length)?data[i]:0x00; return (i >= offset && i < length);}
  

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