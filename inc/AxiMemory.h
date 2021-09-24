/* 
  Jeff Lieu <lieumychuong@gmail.com> 
  Memory model with AXI slave interface 
*/

#pragma once

#include <list>
#include <iterator>
#include "VtbAxiBus.h"
#include "VtbDriver.h"
#include "VtbTransaction.h"
#include "vtb_messages.h"

namespace vtb {

template <typename Type_A>
struct MemorySegment {

  uint32_t size;
  Type_A   base;
  MemorySegment(Type_A base, uint32_t size) : base(base), size(size) {}
  ~MemorySegment(){}
};



template <typename Type_A, typename Type_D>
class AxiMemory : public VtbObject {
  
  enum T_State {
    ST_IDLE,
    ST_DATA,
    ST_RESP,
    ST_ERROR
  };
  
public:

  /* 
    How to use the class:
    + Instantiate the memory and bus: 
      AxiMem32*  axi_memory = new AxiMem32( (4<<10) , 0x0, "Mem 4kB");
      Axi32M2S   axi_dut_to_mem;
      Axi32S2M   axi_mem_to_dut;

    + Randomize data:
      srand(1);
      axi_memory->randomizeData();

    + Connect to DUT and run in the main loop

      while (true)  {
          main_time++;  // Time passes...

          // Toggle a fast (time/2 period) clock        
          top->ap_clk = main_time & 0x1;
          // Evaluate model
          // (If you have multiple models being simulated in the same
          // timestep then instead of eval(), call eval_step() on each, then
          // eval_end_step() on each.)

          if (!top->ap_clk) {
            //Pass signals going into blocks on negative edge
            top->ap_rst_n = (CLOCK_CYCLE > 100);
            
            top->m_axi_a_AWREADY = axi_mem_to_dut.awready; 
            top->m_axi_a_WREADY  = axi_mem_to_dut.wready ;
            ...

            axi_dut_to_mem.awvalid   = top->m_axi_a_AWVALID ;
            axi_dut_to_mem.awaddr    = top->m_axi_a_AWADDR  ;
            axi_dut_to_mem.awid      = top->m_axi_a_AWID    ;
            ...
            
          }

          top->eval();
          axi_memory->eval(main_time, top->ap_clk, (main_time < 100), axi_dut_to_mem, axi_mem_to_dut);
        }

  */

  AxiMemory(const uint32_t size_bytes, const Type_A base_addr = 0, const char* c = nullptr) : base_addr(base_addr), mem_size(size_bytes) {
    
    this->name = (c == nullptr)?"AxiMemory":c;
    mem_array = new uint8_t[mem_size];
    //base address must align to bus_size
    bus_size = sizeof(Type_D);
    ASSERT((base_addr % bus_size) == 0, "Base address must align to bus size");

  };

  ~AxiMemory() {delete mem_array; mem_array = nullptr;};

  
  void eval(uint64_t time, bool clk, bool rst, const AxiBusM2S<Type_A, Type_D> & m2s, AxiBusS2M<Type_A, Type_D>& s2m);
  
  void randomizeData() {  
                          for(int i = 0; i < mem_size; i++) {
                            mem_array[i] = rand();
                          }
                       }
  void*     getMemPtr(uint32_t segment) {return (void*) mem_array;}
  uint32_t  getMemSize(uint32_t segment) {return mem_size;}

private:
  
  uint8_t* mem_array;
  uint32_t mem_size;
  
  AxiBusS2M<Type_A, Type_D> next_output;  
  bool last_clk = 0;

  T_State rd_state = ST_IDLE;
  T_State wr_state = ST_IDLE;  
  Type_A base_addr = 0;
  Type_A waddr;
  uint32_t wlen;
  uint32_t wsize;
  uint32_t wburst;
  uint32_t bus_size;
  Type_A raddr;
  uint32_t rlen;
  uint32_t rsize;
  uint32_t rburst;
  uint64_t time = 0;
  
};



template <typename Type_A, typename Type_D>
void AxiMemory<Type_A, Type_D>::eval(uint64_t time, bool clk, bool rst, const AxiBusM2S<Type_A, Type_D> & m2s, AxiBusS2M<Type_A, Type_D>& s2m) {    
  this->time = time;

  if (clk & ! last_clk) {
    
    if (rst) {
     
      rd_state = T_State::ST_IDLE;
      wr_state = T_State::ST_IDLE;
    
    } else {
      
      //Write channel statemachine
      switch(wr_state) 
      {
        case ST_IDLE :
          if (s2m.awready && m2s.awvalid)
          {
            wr_state = ST_DATA;
            waddr    = m2s.awaddr;
            wlen     = m2s.awlen + 1;
            wsize    = m2s.awsize;
            wburst   = m2s.awburst;

            PRINT_INFO("Write transaction (Addr: 0x%016lx Length: %d) \n", (uint64_t)waddr, wlen);

            uint32_t xfer_len_byte = (1 << wsize)*wlen;
            //Check that transaction fall within the memroy
            ASSERT(((waddr >= base_addr)  && ((waddr + xfer_len_byte) < (base_addr + mem_size))), "Address out of range %lx %ld\n", (uint64_t)waddr, (uint64_t)xfer_len_byte);
            //Narrow transfer not supported
            ASSERT((1<<wsize) == bus_size, "Narrow transfer not suported\n");
            //Only Increment burst is supported
            ASSERT(wburst     == T_AxBurst::INCR, "Burst type %d not supported", uint32_t(wburst));

            waddr = waddr - base_addr;
          }
          break;                
        case ST_DATA :  
          if (s2m.wready && m2s.wvalid)
          {
            
            if (m2s.wlast)
              wr_state = ST_RESP;
            else 
              wr_state = ST_DATA;
                
            for(int byte = 0; byte < bus_size; byte++)
            {
              if ((m2s.wstrb >> byte) & 0x1)
              {
                //Checking address alignment
                ASSERT((waddr % bus_size) == byte, "Address alignment check failed\n");
                mem_array[waddr] = (m2s.wdata >> (8*byte)) & 0xFF;
                waddr++;
              }
            }

          }
          break;
        case ST_RESP :           
          if (m2s.bready && s2m.bvalid)
          {
            wr_state = ST_IDLE;            
          }
          break;
        default : break;
      }
      
      switch(wr_state) 
      {
        case ST_IDLE :  
          next_output.awready = 1;
          next_output.wready  = 0;        
          next_output.bvalid  = 0;
          next_output.bid     = 0;
          next_output.buser   = 0;
          next_output.bresp   = 0;          
          break;                
        case ST_DATA :  
          next_output.awready = 0;
          next_output.wready  = 1;        
          next_output.bvalid  = 0;
          next_output.bid     = 0;
          next_output.buser   = 0;
          next_output.bresp   = 0; 
          break;         
        case ST_RESP : 
          next_output.awready = 0;
          next_output.wready  = 0;        
          next_output.bvalid  = 1;
          next_output.bid     = 0;
          next_output.buser   = 0;
          next_output.bresp   = T_AxResp::OKAY;          
          break;
        default : break;
      }

      //Read channel statemachine
      switch(rd_state) 
      {
        case ST_IDLE :  
          if (s2m.arready && m2s.arvalid)
          {

            raddr    = m2s.araddr;
            rlen     = m2s.arlen + 1;
            rsize    = m2s.arsize;
            rburst   = m2s.arburst;

            uint32_t xfer_len_byte = (1 << rsize)*rlen;
            //Check that transaction fall within the memroy
            ASSERT(((raddr >= base_addr)  && ((raddr + xfer_len_byte) < (base_addr + mem_size))), "Address out of range %lx %ld\n", (uint64_t)raddr, (uint64_t)xfer_len_byte);
            //Narrow transfer not supported
            ASSERT((1<<rsize) == bus_size, "Narrow transfer not suported\n");
            //Only Increment burst is supported
            ASSERT(rburst     == T_AxBurst::INCR, "Burst type %d not supported", rburst);

            PRINT_INFO("Read transaction (Addr: 0x%016lx Length: %d)\n", (uint64_t)raddr, rlen);
            raddr = (raddr - base_addr) & (~(bus_size-1));
            rd_state = ST_DATA;                        
          }
          break;
                
        case ST_DATA :  
          if (s2m.rvalid && m2s.rready)
          {
             
            rd_state = s2m.rlast?ST_IDLE:ST_DATA;
          }
          break;
        default : break;
      }

      switch(rd_state) 
      {
        case ST_IDLE :  
          next_output.rresp   = 0;
          next_output.arready = 1;
          next_output.rvalid  = 0;
          next_output.rlast   = 0;
          next_output.rid     = 0;
          next_output.ruser   = 0;
          next_output.rdata   = 0;
          break;
                
        case ST_DATA :  
          next_output.rresp   = 0;
          next_output.arready = 0;  
          next_output.rvalid  = 1;
          next_output.rlast   = (rlen == 1);
          next_output.rid     = 0;
          next_output.ruser   = 0;
          //Put data on the bus
          next_output.rdata = 0;
          for(int byte = bus_size-1; byte >= 0; byte--)
          {
            next_output.rdata = (next_output.rdata << 8) | mem_array[raddr + byte];
          }           
          raddr += bus_size;
          rlen--;            
          break;
        default : break;
      }



      s2m = next_output;
    }
  }
  last_clk = clk;
};

}
