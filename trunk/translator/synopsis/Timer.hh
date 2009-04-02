//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//

#ifndef Synopsis_Timer_hh_
#define Synopsis_Timer_hh_

#include <ctime>

namespace Synopsis
{

class Timer
{
public:
  Timer() : my_start(std::clock()) {}
  double elapsed() const { return  double(std::clock() - my_start) / CLOCKS_PER_SEC;}
private:
  std::clock_t my_start;
};

}

#endif
