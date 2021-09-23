/* 
  Jeff Lieu <lieumychuong@gmail.com>  
*/

#pragma once

#include <list>
#include <iterator>
#include "AxiBus.h"
#include "Driver.h"
#include "Transaction.h"

namespace vtb {


template <typename Type_D>
class AxiStreamDriver : public Driver< AxiStreamTransaction > {

  using Transaction = AxiStreamTransaction;
  using AxiSource   = AxiStreamSource<Type_D>;
  using AxiSink     = AxiStreamSink;
  using Driver<Transaction> :: transQueue;

  enum T_State {    
    ST_IDLE,    
    ST_GENERATE,
    ST_FETCH
  };

public:
  AxiStreamDriver(const char* c = nullptr) {this->name = (c == nullptr)?"AXI Stream driver":c;};
  ~AxiStreamDriver();

  void pushTransaction(Transaction & t) { transQueue.push_back(t);}
  void eval(uint64_t time, bool clk, bool rst,  AxiSource &src, const AxiSink &sink);

private: 
  const char* name;
  AxiSource next_output;
  T_State state = ST_IDLE;
  uint64_t trans_cnt  = 0;
  uint64_t rd_addr    = 0;
  uint64_t byte_cnt   = 0;
  uint64_t total_beat = 0;
  uint64_t beat_cnt   = 0;
  bool     first_beat = 0;  
  bool     last_clk;
  uint64_t time;

};

template <typename Type_D>
void AxiStreamDriver<Type_D>::eval(uint64_t time, bool clk, bool rst,  AxiSource &src, const AxiSink &sink) {    
  this->time = time;
  if (clk & ! last_clk) {
    if (rst)
      state = T_State::ST_IDLE;
    else {      
      auto next_transaction = transQueue.front();      
      switch(state) 
      {
        case ST_IDLE    : if (!transQueue.empty()) {
                            printf("%s : Time %08ld: %s \n", this->name, time, next_transaction.getInfo().c_str());                                                    
                            if (time >= next_transaction.timestamp) {
                              state = ST_FETCH;                                                        
                              trans_cnt ++;
                            }                            
                            //Calculating beats 
                            total_beat = ((next_transaction.length + next_transaction.offset) + (sizeof(Type_D)) - 1)/(sizeof(Type_D));                            
                            rd_addr    = 0;
                            first_beat = true;
                          }
                          break;

        case ST_FETCH   : if (src.tvalid && sink.tready) {
                            PRINT_INFO("Beat: %ld\n", total_beat);                            
                            rd_addr += sizeof(Type_D);
                            total_beat--;
                            if (total_beat == 0) 
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
                          for (uint32_t i  = 0; i < sizeof(Type_D); i++) {             
                              uint8_t byte;                 
                              setBit<uint64_t>(&next_output.tkeep, i, (next_transaction.getByte(rd_addr+i, &byte)));
                              putBytesToBus<Type_D>( &next_output.tdata, byte, i);
                            }
                          next_output.tvalid = 1;                          
                          next_output.tlast  = (total_beat == 1);
                          first_beat = false;
                        break;          
        default : break;
      }
      
      src = next_output;    
    }
  }
  last_clk = clk;
};


}