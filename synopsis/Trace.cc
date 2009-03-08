//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//

#include <Synopsis/Trace.hh>

using namespace Synopsis;

unsigned int Trace::my_mask = Trace::NONE;
size_t Trace::my_level = 0;
