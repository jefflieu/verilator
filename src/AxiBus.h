


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

};
