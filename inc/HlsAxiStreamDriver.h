/* 
  Jeff Lieu <lieumychuong@gmail.com>  
*/

#pragma once

#include <list>
#include <iterator>
#include "VtbAxiBus.h"
#include "VtbDriver.h"
#include "AxiStreamDriver.h"
#include "VtbTransaction.h"
#include "hls_stream.h"

namespace vtb {


template <typename Type_HlsAxiStream, typename T_VtbBus>
class HlsAxiStreamDriver : public AxiStreamDriver<T_VtbBus> {

  using Transaction = AxiStreamTransaction<T_VtbBus>;
  using AxiSource   = AxiStreamSource<T_VtbBus>;
  using AxiSink     = AxiStreamSink;  
  //using Driver<Transaction> :: transQueue;


public:
  HlsAxiStreamDriver(T_VtbBus (*packFunction)(const Type_HlsAxiStream& data), const char* c = nullptr) : AxiStreamDriver<T_VtbBus>(c) {this->packHlsAxiStreamData = packFunction;}
  ~HlsAxiStreamDriver(){};
  
  //This write never blocks
  bool write(const Type_HlsAxiStream& stream_data) {
    //Converting data item to a a stream beat
    Transaction* t = new AxiStreamBeat<T_VtbBus>(packHlsAxiStreamData(stream_data), stream_data.keep, stream_data.strb, stream_data.last);
    this->pushTransaction(*t);
  }

  T_VtbBus (*packHlsAxiStreamData)(const Type_HlsAxiStream& data);

private: 


};


}