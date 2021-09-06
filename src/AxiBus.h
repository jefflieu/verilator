


#pragma once

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

  AxiLiteM2S():awvalid(0), awaddr(0), wvalid(0), wdata(0), wstrb(0), bready(0), arvalid(0), araddr(0), rready(0) {};

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

  AxiLiteS2M():awready(0), wready(0), bvalid(0), bresp(0), rresp(0), arready(0), rvalid(0), rdata(0) {};
};
