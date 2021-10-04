/* 
  Jeff Lieu <lieumychuong@gmail.com>  
*/

#pragma once

#include <list>
#include <iterator>
#include "VtbAxiBus.h"
#include "VtbDriver.h"
#include "VtbTransaction.h"

namespace vtb {


template <typename T_VtbBus>
class AxiStreamDriver : public Driver< AxiStreamTransaction<T_VtbBus> > {

  using Transaction = AxiStreamTransaction<T_VtbBus>;
  using AxiSource   = AxiStreamSource<T_VtbBus>;
  using AxiSink     = AxiStreamSink;
  using Driver<Transaction> :: transQueue;

  enum T_State {    
    ST_IDLE,    
    ST_GENERATE,
    ST_FETCH
  };

public:
  AxiStreamDriver(const char* c = nullptr) {this->name = (c == nullptr)?"AXI Stream driver":c;}
  ~AxiStreamDriver(){};

  void pushTransaction(Transaction & t) { transQueue.push_back(t);}
  void eval(uint64_t time, bool clk, bool rst,  AxiSource &src, const AxiSink &sink);
  void drivePacket(uint64_t time, bool clk, bool rst,  AxiSource &src, const AxiSink &sink);
  void driveBeat(uint64_t time, bool clk, bool rst,  AxiSource &src, const AxiSink &sink);
  void driveNone(AxiSource &src);
private: 

  AxiSource next_output;
  T_State state = ST_IDLE;
  uint64_t trans_cnt  = 0;
  uint64_t rd_addr    = 0;  
  uint64_t remaining_beat = 0;  
  bool     last_clk;
  uint64_t time;
};

template <typename T_VtbBus>
void AxiStreamDriver<T_VtbBus>::eval(uint64_t time, bool clk, bool rst,  AxiSource &src, const AxiSink &sink) {    
  this->time = time;
  if (clk & ! last_clk) {
    if (rst) {
      state = T_State::ST_IDLE;
      driveNone(src);
    } else {      
      Transaction& queue_head = transQueue.front();      
      if (!transQueue.empty()) {        
        if (queue_head.type == Transaction::ONE_PACKET)
          drivePacket(time, clk, rst, src, sink);
        else if (queue_head.type == Transaction::SINGLE_BEAT)          
          driveBeat(time, clk, rst, src, sink);      
      } else {
        state = T_State::ST_IDLE;
        driveNone(src);
      }
    }
  }
  last_clk = clk;
};

template <typename T_VtbBus>
void AxiStreamDriver<T_VtbBus>::drivePacket(uint64_t time, bool clk, bool rst,  AxiSource &src, const AxiSink &sink) {
  Transaction& queue_head = transQueue.front();
  switch(state) 
  {
    case ST_IDLE    : if (!transQueue.empty()) {
                        printf("%s : Time %08ld: %s \n", this->name, time, queue_head.getInfo().c_str());                                                    
                        if (time >= queue_head.timestamp) {
                          state = ST_FETCH;                                                        
                          trans_cnt ++;
                        }                            
                        //Calculating beats                             
                        remaining_beat = ((queue_head.length + queue_head.offset) + (sizeof(T_VtbBus)) - 1)/(sizeof(T_VtbBus));                            
                        rd_addr    = 0;
                      }
                      break;

    case ST_FETCH   : if (src.tvalid && sink.tready) {
                        PRINT_INFO("Beat: %ld\n", remaining_beat);                            
                        rd_addr += sizeof(T_VtbBus);
                        remaining_beat--;
                        if (remaining_beat == 0) 
                          {
                            state = ST_IDLE;
                            transQueue.pop_front();
                          }
                      }
                      break;          
    default : break;
  }

  switch(state) 
  {
    case ST_IDLE    : next_output.tdata  = 0;
                      next_output.tvalid = 0;
                      next_output.tlast  = 0;
                      break;

    case ST_FETCH   : 
                      next_output.tdata = 0;                          
                      next_output.tkeep = 0;
                      next_output.tstrb  = 0;
                      for (uint32_t i  = 0; i < sizeof(T_VtbBus); i++) {             
                          uint8_t byte;                 
                          setBit<decltype(next_output.tkeep)>(&next_output.tkeep, i, (queue_head.getByte(rd_addr+i, &byte)));
                          setByte<decltype(next_output.tdata)>( &next_output.tdata, byte, i);
                        }
                      next_output.tstrb  = next_output.tkeep;
                      next_output.tvalid = 1;                          
                      next_output.tlast  = (remaining_beat == 1);                          
                    break;          
    default : break;
  }
  src = next_output;
};

template <typename T_VtbBus>
void AxiStreamDriver<T_VtbBus>::driveBeat(uint64_t time, bool clk, bool rst,  AxiSource &src, const AxiSink &sink) {
  if (src.tvalid && sink.tready) {
    transQueue.pop_front();
  }
  if (!transQueue.empty()) {
    Transaction& queue_head = transQueue.front();
    next_output.tvalid = (time >= queue_head.timestamp);
    next_output.tstrb  = queue_head.strb;
    next_output.tkeep  = queue_head.keep;
    next_output.tlast  = queue_head.last;
    for (uint32_t i  = 0; i < sizeof(T_VtbBus); i++) {             
      uint8_t byte;                 
      queue_head.getByte(i, &byte);
      setByte<decltype(next_output.tdata)>( &next_output.tdata, byte, i);
    }
  } else {
    next_output.tvalid = 0;
    next_output.tstrb  = 0;
    next_output.tkeep  = 0;
    next_output.tlast  = 0;
    for (uint32_t i  = 0; i < sizeof(T_VtbBus); i++) {             
      setByte<decltype(next_output.tdata)>( &next_output.tdata, 0, i);
    }
  }
  src = next_output;
};

template <typename T_VtbBus>
void AxiStreamDriver<T_VtbBus>::driveNone(AxiSource& src)
{
  src.tdata  = 0;
  src.tvalid = 0;
  src.tlast  = 0;
  src.tkeep  = 0;
  src.tstrb  = 0;
};

}