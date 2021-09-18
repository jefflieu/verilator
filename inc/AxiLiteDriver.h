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
    ST_RDATA_POLL,
  };
public:

  /* How to use the class:
    
    Create Driver and bus:
      AxiLite32* axiliteDriver = new AxiLite32();
      AxiLite32M2S axi_driver_to_dut;
      AxiLite32S2M axi_dut_to_driver;
      
    Add transaction to Driver
      AxiLiteTransaction* t;    
      t = new AxiLiteTransaction(AxiLiteTransaction::WRITE, 0x10, a);
      axiliteDriver->pushTransaction(*t);

    Connect driver to dut and call eval in the simulation loop of Verilator

      if (!top->ap_clk) {

        //Pass signals going into blocks on negative edge
        top->s_axi_BUS_A_AWVALID = axi_driver_to_dut.awvalid;
        top->s_axi_BUS_A_AWADDR  = axi_driver_to_dut.awaddr;
        top->s_axi_BUS_A_WVALID  = axi_driver_to_dut.wvalid;
        top->s_axi_BUS_A_WDATA   = axi_driver_to_dut.wdata;
        top->s_axi_BUS_A_WSTRB   = axi_driver_to_dut.wstrb;
        top->s_axi_BUS_A_ARVALID = axi_driver_to_dut.arvalid;
        top->s_axi_BUS_A_ARADDR  = axi_driver_to_dut.araddr;
        top->s_axi_BUS_A_RREADY  = axi_driver_to_dut.rready;
        top->s_axi_BUS_A_BREADY  = axi_driver_to_dut.bready;


        axi_dut_to_driver.awready = top->s_axi_BUS_A_AWREADY;
        axi_dut_to_driver.arready = top->s_axi_BUS_A_ARREADY;
        axi_dut_to_driver.wready  = top->s_axi_BUS_A_WREADY;
        axi_dut_to_driver.rvalid  = top->s_axi_BUS_A_RVALID ;
        axi_dut_to_driver.rdata   = top->s_axi_BUS_A_RDATA  ;
        axi_dut_to_driver.bvalid  = top->s_axi_BUS_A_BVALID;
        axi_dut_to_driver.bresp   = top->s_axi_BUS_A_BRESP;
        axi_dut_to_driver.rresp   = top->s_axi_BUS_A_RRESP;          
      }

      top->eval();
      axiliteDriver->eval(main_time, top->ap_clk, (main_time < 100), axi_driver_to_dut, axi_dut_to_driver);

  */

  AxiLiteDriver(const char* c = nullptr) {this->name = (c == nullptr)?"AxiLiteDriver":c;};
  ~AxiLiteDriver() {};

  void pushTransaction(AxiLiteTransaction & t) { transQueue.push_back(t);}
  void eval(uint64_t time, bool clk, bool rst, AxiLiteM2S<Type_A, Type_D> & m2s, const AxiLiteS2M<Type_A, Type_D>& s2m);
  int getTransactionCount() {return transQueue.size();};

private:
  const char* name;
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
                          printf("%s : Time %08ld: %s \n", this->name, time, next_transaction.getInfo().c_str());
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
                        } else if (state == ST_RADDR) {
                          if (m2s.arvalid && s2m.arready) {
                            next_output.arvalid = 0;
                            state = ST_RDATA;                             
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
        case ST_RDATA_POLL:  
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
                          
                          switch(next_transaction.type) {
                          case AxiLiteTransaction::READ            :  state = ST_IDLE;
                                                                      transQueue.pop_front();
                                                                      break;
                          case AxiLiteTransaction::READ_CHECK      :  printf("%s : Checking data %s (Expect: 0x%016lx Actual: 0x%016lx)\r\n", name, ((read_data & next_transaction.mask) == (next_transaction.data & next_transaction.mask))?"OK":"FAILED", next_transaction.data & next_transaction.mask, read_data & next_transaction.mask);
                                                                      state = ST_IDLE;
                                                                      transQueue.pop_front();
                                                                      break;
                          case AxiLiteTransaction::READ_MOD_WRITE  :  break;
                          case AxiLiteTransaction::READ_POLL       :  if ((read_data & next_transaction.mask) == next_transaction.data) {
                                                                        state = ST_IDLE;
                                                                        transQueue.pop_front();
                                                                      } else {
                                                                        state = ST_RADDR;
                                                                      }
                                                                      break;
                          
                          default : break;
                          }
                          
                        }
                        break;          
        default : break;
      }
      if (m2s.bready && s2m.bvalid) {
        switch(s2m.bresp) 
        {
          case 0 : printf("%s : Write transaction OK\r\n", name); break;
          default: printf("%s : Write transaction failed\r\n", name); break;
        }
      }

      m2s = next_output;
    }
  }
  last_clk = clk;
};

}
