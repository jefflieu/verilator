/* 
  Jeff Lieu <lieumychuong@gmail.com>  
*/

#pragma once

#include <list>
#include <iterator>
#include <vector>
#include "VtbAxiBus.h"
#include "VtbDriver.h"
#include "VtbTransaction.h"

namespace vtb {


template <typename T_VtbBus>
class AxiStreamReceiver : public Driver< AxiStreamTransaction <T_VtbBus> > {

  using ByteVector  = std::vector<uint8_t>;
  using Transaction = AxiStreamTransaction<T_VtbBus>;
  using AxiSource   = AxiStreamSource<T_VtbBus>;
  using AxiSink     = AxiStreamSink;
  using Driver<Transaction> :: transQueue;

  enum T_State {    
    ST_IDLE,    
    ST_RUN
  };

public:
  AxiStreamReceiver(const char* c = nullptr, bool mode = true) {this->name = (c == nullptr)?"AXI Stream receiver":c; this->packet_mode = mode;}
  ~AxiStreamReceiver();

  bool popTransaction(Transaction & t) { if (transQueue.empty()) return false; else t = transQueue.front(); transQueue.pop_front(); return true;}

  void eval(uint64_t time, bool clk, bool rst,  const AxiSource &src, AxiSink &sink);
  void reset() {state = T_State::ST_IDLE; total_beat = 0; first_beat = true; complete = false; offset = 0; local_buffer.clear();}

private: 
  ByteVector local_buffer;
  AxiSink  next_output;
  T_State state = ST_IDLE;
  uint64_t trans_cnt  = 0;  
  uint64_t total_beat = 0;  
  bool     first_beat = 0;  
  bool     complete   = 0;
  uint32_t     offset = 0;
  bool     last_clk;
  uint64_t time;
  bool     packet_mode;
};

template <typename T_VtbBus>
void AxiStreamReceiver<T_VtbBus>::eval(uint64_t time, bool clk, bool rst,  const AxiSource &src, AxiSink &sink) {    
  this->time = time;
  if (clk & ! last_clk) {
    if (rst)
      this->reset();
    else {      
      
      switch(state) 
      {
        case ST_IDLE    : 
                          next_output.tready = true;
                          if (src.tvalid && sink.tready) {
                                    
                            for(uint32_t i = 0; i < sizeof(T_VtbBus); i++) {
                              if (src.isValidByte(i))
                                local_buffer.push_back(src.getByte(i));
                              else if (first_beat) offset++;
                            }

                            first_beat = false;
                            complete = src.tlast;
                            total_beat++;
                            PRINT_INFO("Receiving beat %ld %s\n", total_beat, complete?"done":" ");
                            if (packet_mode) {
                              if (complete) {
                                PRINT_INFO("Received a stream of %ld beats, %ld bytes\n", total_beat, local_buffer.size());                              
                                Transaction *t = new AxiStreamPacket<T_VtbBus>(local_buffer.data(), total_beat*sizeof(T_VtbBus));
                                transQueue.push_back(*t);
                              }
                            } else {
                              Transaction *t = new AxiStreamBeat<T_VtbBus>(src.tdata, src.tkeep, src.tstrb, src.tlast);
                              transQueue.push_back(*t);
                              PRINT_INFO("Created beat\n");
                            }

                            if (complete)                            
                              this->reset();
                          }
                          break;
        case ST_RUN     : if (src.tvalid && sink.tready) {
                            
                          }
                          break;          
        default : break;
      }

      switch(state) 
      {
        case ST_IDLE    : break;
        case ST_RUN     : break;          
        default : break;
      }      
      sink = next_output;
    }
  }
  last_clk = clk;
};


}