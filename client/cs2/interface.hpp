#pragma once

namespace cs2 {

struct intfreg {
  void * (*create)(void);
  const char * name;
  intfreg    * next;
};

}
