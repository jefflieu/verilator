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
class HlsAxiStreamReceiver : public AxiStreamReceiver<T_VtbBus> {

  using Transaction = AxiStreamTransaction<T_VtbBus>;
  using AxiSource   = AxiStreamSource<T_VtbBus>;
  using AxiSink     = AxiStreamSink;  
  //using Driver<Transaction> :: transQueue;


public:
  HlsAxiStreamReceiver(Type_HlsAxiStream (*unpackFunction)(const T_VtbBus& data), const char* c = nullptr) : AxiStreamReceiver<T_VtbBus>(c, false) {this->unpackHlsAxiStreamData = unpackFunction;}
  ~HlsAxiStreamReceiver(){};
  
  //This write never blocks
  Type_HlsAxiStream read() {
    //Converting data item to a a stream beat
    Transaction *t = new AxiStreamBeat<T_VtbBus>();
    Type_HlsAxiStream stream_data;    
    if (this->popTransaction(*t)) {
      stream_data = unpackHlsAxiStreamData(t->getBusData());
      stream_data.keep = t->keep;
      stream_data.strb = t->strb;
      stream_data.last = t->last;
    }
    return stream_data;
  }

  Type_HlsAxiStream (*unpackHlsAxiStreamData)(const T_VtbBus & data);

private: 


};


}