

#pragma once

#include <list>
#include <iterator>
#include "AxiBus.h"
#include "Driver.h"
#include "Transaction.h"


template <typename Type_A, typename Type_D>
class AxiLiteDriver : Driver<AxiLiteTransaction> {
  enum T_State {
    ST_IDLE,
    ST_WAIT,
    ST_WADDR,
    ST_WDATA,
    ST_RADDR,
    ST_RDATA, 
    ST_RDATA_CHK,
    ST_RDATA_MOD_WRITE,
  };
public:
  AxiLiteDriver() {};
  ~AxiLiteDriver() {};

  void pushTransaction(AxiLiteTransaction & t) { transQueue.push_back(t);}
  void eval(uint64_t time, bool clk, bool rst, AxiLiteM2S<Type_A, Type_D> & m2s, const AxiLiteS2M<Type_A, Type_D>& s2m);

private:
  AxiLiteM2S<Type_A, Type_D> next_output;
  bool last_clk = 0;
  T_State state = ST_IDLE;
  int trans_cnt = 0;
  AxiLiteTransaction* current_transaction;
  uint64_t read_data;

};



template <typename Type_A, typename Type_D>
void AxiLiteDriver<Type_A, Type_D>::eval(uint64_t time, bool clk, bool rst, AxiLiteM2S<Type_A, Type_D> & m2s, const AxiLiteS2M<Type_A, Type_D>& s2m) {    
  if (clk & ! last_clk) {
    if (rst) 
      state = T_State::ST_IDLE;
    else {

      auto next_transaction = transQueue.front();
      switch(state) 
      {
        case ST_IDLE :  next_output.awvalid = 0;
                        next_output.awaddr  = 0x0;
                        next_output.wvalid  = 0;
                        next_output.wdata   = 0;
                        next_output.wstrb   = 0;
                        next_output.arvalid = 0;
                        next_output.araddr  = 0;
                        next_output.rready  = 0;
                        next_output.bready  = 1;                                                                              
                        
                        if (!transQueue.empty()) {
                          state = ST_WAIT;
                          trans_cnt ++;
                        }
                        
                        break;

        case ST_WAIT :  if (time >= next_transaction.timestamp)
                          if (next_transaction.type == AxiLiteTransaction::WRITE)                              
                            state = ST_WADDR;
                          else 
                            state = ST_RADDR;
                        break;
        case ST_WADDR:  
        case ST_RADDR:  
                        next_output.awvalid = (state == ST_WADDR);
                        next_output.awaddr  = next_transaction.addr;
                        next_output.wvalid  = 0;
                        next_output.wdata   = next_transaction.data;
                        next_output.wstrb   = 0x0;
                        next_output.arvalid = (state == ST_RADDR);
                        next_output.araddr  = next_transaction.addr;
                        next_output.rready  = 0;
                        next_output.bready  = 1;
                        if (state == ST_WADDR){
                          if (m2s.awvalid && s2m.awready) {
                            next_output.awvalid = 0;
                            state = ST_WDATA;
                          }
                        }
                        else if (state == ST_RADDR) {
                          if (m2s.arvalid && s2m.arready) {
                            next_output.arvalid = 0;
                            state = ST_RDATA; 
                            switch(next_transaction.type) {
                            case AxiLiteTransaction::READ            : state = ST_RDATA; break;
                            case AxiLiteTransaction::READ_CHECK      : state = ST_RDATA_CHK; break;
                            case AxiLiteTransaction::READ_MOD_WRITE  : state = ST_RDATA_MOD_WRITE; break;
                            default     : break;
                            }
                          }
                        }
                          
                        break;
        case ST_WDATA:  next_output.awvalid = 0;
                        next_output.awaddr  = 0;
                        next_output.wvalid  = 1;
                        next_output.wdata   = next_transaction.data;
                        next_output.wstrb   = 0xFFFFFFFF;
                        next_output.arvalid = 0;
                        next_output.araddr  = 0;
                        next_output.rready  = 0;
                        next_output.bready  = 1;
                        if (m2s.wvalid && s2m.wready) {
                          next_output.wvalid = 0;
                          state = ST_IDLE;
                          transQueue.pop_front();
                        }
                        break;
        case ST_RDATA_MOD_WRITE:  
        case ST_RDATA_CHK:  
        case ST_RDATA:  next_output.awvalid = 0;
                        next_output.awaddr  = 0;
                        next_output.wvalid  = 0;
                        next_output.wdata   = 0;
                        next_output.wstrb   = 0;
                        next_output.arvalid = 0;
                        next_output.araddr  = 0;
                        next_output.rready  = 1;
                        next_output.bready  = 1;
                        if (m2s.rready && s2m.rvalid) {
                          next_output.rready = 0;
                          read_data = s2m.rdata;
                          transQueue.pop_front();
                          if (state == ST_RDATA_CHK)
                          {
                            printf("Checking data %s (Expect: 0x%016lx Actual: 0x%016lx)\r\n", ((read_data & next_transaction.mask) == (next_transaction.data & next_transaction.mask))?"OK":"FAILED", next_transaction.data & next_transaction.mask, read_data & next_transaction.mask);
                          }
                          state = ST_IDLE;
                        }
                        break;          
        default : break;
      }
      if (m2s.bready && s2m.bvalid) {
        switch(s2m.bresp) 
        {
          case 0 : printf("Write transaction OK\r\n"); break;
          default: printf("Write transaction failed\r\n"); break;
        }
      }
      m2s = next_output;
    }
  }
  last_clk = clk;
};

