


#pragma once

#include "vtb_messages.h"
#include "VtbObject.h"

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

template <typename T_VtbBus> 
void inline setByte(T_VtbBus* bus, uint8_t byte, uint32_t i) {
  if (sizeof(T_VtbBus) <= 8)
  {
    T_VtbBus tmp = *bus;
    tmp &= ~(0xffull << (i*8));
    tmp |= (unsigned long long) (byte &  0xff) << (i*8);
    *bus = tmp;
  } else {
    ASSERT_CODE(0, "Not supported bus type\n");
  }
}

template <typename T_VtbBus>
void inline setBit(T_VtbBus* a, uint32_t i, uint32_t bit) {
  if (sizeof(T_VtbBus) <= 8)
  {
    T_VtbBus tmp = (*a) & (~(1 << i));
    tmp |= ((bit) & 0x1) << i;    
    *a = tmp;
  } else {
    ASSERT_CODE(0, "Not supported bus type\n");
  } 
}

bool inline getBit(const uint64_t a, uint32_t i) {
  return ((a & (1ull << i)) != 0);  
}

bool inline getBit(const uint32_t* a, uint32_t i) {
  return 0;
}


uint8_t inline getByte(const uint64_t a, uint32_t i) {
  return (a >> (i*8)) & 0xff;  
}

uint8_t inline getByte(const uint32_t* a, uint32_t i) {
  return 0;
}


template <typename T_VtbAddress, typename T_VtbBus> struct AxiLiteM2S : public VtbObject {

  bool awvalid;
  T_VtbAddress awaddr;
  bool wvalid;
  T_VtbBus wdata;
  uint32_t wstrb;  
  bool bready;

  bool arvalid;
  T_VtbAddress araddr;
  bool rready;

  //AxiLiteM2S():awvalid(0), awaddr(0), wvalid(0), wdata(0), wstrb(0), bready(0), arvalid(0), araddr(0), rready(0) {};
  AxiLiteM2S() {memset(this, 0, sizeof(AxiLiteM2S<T_VtbAddress, T_VtbBus>));}

};

template <typename T_VtbAddress, typename T_VtbBus> struct AxiLiteS2M : public VtbObject {

  bool awready;
  
  bool wready;
  
  bool bvalid;
  uint32_t bresp;
  uint32_t rresp;

  bool arready;  
  bool rvalid;
  T_VtbBus rdata;

  AxiLiteS2M() {memset(this, 0, sizeof(AxiLiteM2S<T_VtbAddress, T_VtbBus>));}
};


template <typename T_VtbAddress, typename T_VtbBus> struct AxiBusM2S : public VtbObject {
  
  bool awvalid;
  T_VtbAddress awaddr;
  bool wvalid;
  T_VtbBus wdata;
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
  T_VtbAddress araddr;
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

  AxiBusM2S() {memset(this, 0, sizeof(AxiBusM2S<T_VtbAddress, T_VtbBus>));}
};


template <typename T_VtbAddress, typename T_VtbBus> struct AxiBusS2M : public VtbObject {

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
  T_VtbBus rdata;

  AxiBusS2M() {memset(this, 0, sizeof(AxiBusS2M<T_VtbAddress, T_VtbBus>));}
};

template <typename T_VtbBus> struct AxiStreamSource : public VtbObject {

  bool tvalid;  
  T_VtbBus tdata;
  uint64_t tkeep;
  uint64_t tstrb;
  uint32_t tlast;
  uint32_t tuser;
  uint32_t tid;
  uint32_t tdest;
  bool ignore_keep;
  bool ignore_strb;
  AxiStreamSource() {memset(this, 0, sizeof(AxiStreamSource<T_VtbBus>)); this->ignore_keep = true; this->ignore_strb = true;}
  bool isValidByte(uint32_t i ) const {return (this->ignore_strb || getBit(this->tstrb, 1)) && (this->ignore_keep || getBit(this->tkeep, 1));}
  uint8_t getByte (uint32_t i) const {return vtb::getByte(this->tdata, i);}
  bool setByte (uint8_t data, uint32_t i) const {return vtb::setByte(&this->tdata, data, i);}
};

struct AxiStreamSink : public VtbObject {
  bool tready;
  AxiStreamSink() {this->tready = 0;}
};

}