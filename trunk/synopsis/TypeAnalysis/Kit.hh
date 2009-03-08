//
// Copyright (C) 2005 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_TypeAnalysis_Kit_hh_
#define Synopsis_TypeAnalysis_Kit_hh_

#include <Synopsis/TypeAnalysis/Type.hh>

namespace Synopsis
{
namespace TypeAnalysis
{

//. creates and remembers declared types.
class Kit
{
public:
  Kit();

  Type const *builtin(std::string const &name);
  Type const *enum_(std::string const &name);
  Type const *class_(std::string const &name);
  Type const *union_(std::string const &name);
  Type const *pointer(Type const *type);
  Type const *reference(Type const *type);
  Type const *array(Type const *type);
  Type const *pointer_to_member(Type const *container, Type const *member);
private:
};

}
}

#endif
