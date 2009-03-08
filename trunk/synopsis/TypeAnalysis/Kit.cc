//
// Copyright (C) 2005 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//

#include "Kit.hh"

using namespace Synopsis;
using namespace TypeAnalysis;

Kit::Kit()
{
}

Type const *Kit::builtin(std::string const &)
{
  return 0;
}

Type const *Kit::enum_(std::string const &)
{
  return 0;
}

Type const *Kit::class_(std::string const &)
{
  return 0;
}

Type const *Kit::union_(std::string const &)
{
  return 0;
}

Type const *Kit::pointer(Type const *)
{
  return 0;
}

Type const *Kit::reference(Type const *)
{
  return 0;
}

Type const *Kit::array(Type const *)
{
  return 0;
}

Type const *Kit::pointer_to_member(Type const *, Type const *)
{
  return 0;
}
