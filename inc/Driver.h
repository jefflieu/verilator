

#pragma once

#include <list>
#include <iterator>

namespace vtb {

template <typename TransactionT>
class Driver {

public: 
  Driver() {};
  ~Driver() {};

  std::list<TransactionT> transQueue;

private:




};

}
