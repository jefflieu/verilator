

#pragma once

#include <list>
#include <iterator>

#include "VtbObject.h"

namespace vtb {

template <typename TransactionT>
class Driver : public VtbObject {

protected: 
  Driver() {};  
  ~Driver() {};

  std::list<TransactionT> transQueue;

private:


};

}
