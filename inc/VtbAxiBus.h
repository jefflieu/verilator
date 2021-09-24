


#pragma once

#include "vtb_messages.h"

namespace vtb {

enum T_AxSize{
    _1_BYTE = 0,
    _2_BYTE = 1,
    _4_BYTE = 2,
    _8_BYTE = 3,
   _16_BYTE = 4,
   _32_BYTE = 5,
   _64_BYTE = 6,
  _128_BYTE = 7
};

enum T_AxBurst {
  FIXED  = 0,
  INCR   = 1,
  WRAP   = 2,
  RSVD   = 3
};

enum T_AxResp {
  OKAY = 0,
  EXOKAY = 1,
  SLVERR = 2,
  DECERR = 3
};

template <typename Type_D> 
void inline putBytesToBus(Type_D* bus, uint8_t byte, uint32_t i) {
  if (sizeof(Type_D) <= 8)
  {
    Type_D tmp = *bus;
    tmp &= ~(0xffull << (i*8));
    tmp |= (unsigned long long) (byte &  0xff) << (i*8);
    *bus = tmp;
  } else {
    ASSERT_CODE(0, "Not supported bus type\n");
  }
}

template <typename Type_D>
void inline setBit(Type_D* a, uint8_t i, uint32_t bit) {
  if (sizeof(Type_D) <= 8)
  {
    Type_D tmp = (*a) & (~(1 << i));
    tmp |= ((bit) & 0x1) << i;    
    *a = tmp;
  } else {
    ASSERT_CODE(0, "Not supported bus type\n");
  } 
}


template <typename Type_A, typename Type_D> struct AxiLiteM2S {

  bool awvalid;
  Type_A awaddr;
  bool wvalid;
  Type_D wdata;
  uint32_t wstrb;  
  bool bready;

  bool arvalid;
  Type_A araddr;
  bool rready;

  //AxiLiteM2S():awvalid(0), awaddr(0), wvalid(0), wdata(0), wstrb(0), bready(0), arvalid(0), araddr(0), rready(0) {};
  AxiLiteM2S() {memset(this, 0, sizeof(AxiLiteM2S<Type_A, Type_D>));}

};

template <typename Type_A, typename Type_D> struct AxiLiteS2M {

  bool awready;
  
  bool wready;
  
  bool bvalid;
  uint32_t bresp;
  uint32_t rresp;

  bool arready;  
  bool rvalid;
  Type_D rdata;

  AxiLiteS2M() {memset(this, 0, sizeof(AxiLiteM2S<Type_A, Type_D>));}
};


template <typename Type_A, typename Type_D> struct AxiBusM2S {
  
  bool awvalid;
  Type_A awaddr;
  bool wvalid;
  Type_D wdata;
  bool wlast;
  uint64_t wstrb;  
  uint32_t awid;
  uint32_t awlen;
  uint32_t awsize;
  uint32_t awburst;
  uint32_t awlock;
  uint32_t awcache;
  uint32_t awprot;
  uint32_t awqos;
  uint32_t awregion;
  uint32_t awuser;
  uint32_t wid;
  uint32_t wuser;

  bool bready;

  bool arvalid;
  Type_A araddr;
  uint32_t arid;
  uint32_t arlen;
  uint32_t arsize;
  uint32_t arburst;
  uint32_t arlock;
  uint32_t arcache;
  uint32_t arprot;
  uint32_t arqos;
  uint32_t arregion;
  uint32_t aruser;
  bool rready;

  AxiBusM2S() {memset(this, 0, sizeof(AxiBusM2S<Type_A, Type_D>));}
};


template <typename Type_A, typename Type_D> struct AxiBusS2M {

  bool awready;
  
  bool wready;
  
  bool bvalid;
  uint32_t bid;
  uint32_t buser;
  uint32_t bresp;
  uint32_t rresp;

  bool arready;  
  bool rvalid;
  bool rlast;
  uint32_t rid;
  uint32_t ruser;
  Type_D rdata;

  AxiBusS2M() {memset(this, 0, sizeof(AxiBusS2M<Type_A, Type_D>));}
};

template <typename Type_D> struct AxiStreamSource {

  bool tvalid;  
  Type_D tdata;
  uint64_t tkeep;
  uint64_t tstrb;
  uint32_t tlast;
  uint32_t tuser;
  uint32_t tid;
  uint32_t tdest;

  AxiStreamSource() {memset(this, 0, sizeof(AxiStreamSource<Type_D>));}
};

struct AxiStreamSink {
  bool tready;
  AxiStreamSink() {this->tready = 0;}
};

}