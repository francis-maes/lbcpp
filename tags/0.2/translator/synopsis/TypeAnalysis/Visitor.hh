//
// Copyright (C) 2005 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_TypeAnalysis_Visitor_hh_
#define Synopsis_TypeAnalysis_Visitor_hh_

namespace Synopsis
{
namespace TypeAnalysis
{

class Type;
class BuiltinType;
class Enum;
class Class;
class Union;
class CVType;
class Pointer;
class Reference;
class Array;
class Function;
class PointerToMember;

class Visitor
{
public:
  virtual ~Visitor() {}
  // FIXME: may these use const arguments ?
  virtual void visit(Type *) = 0;
  virtual void visit(BuiltinType *) = 0;
  virtual void visit(Enum *) = 0;
  virtual void visit(Class *) = 0;
  virtual void visit(Union *) = 0;

  virtual void visit(CVType *) = 0;
  virtual void visit(Pointer *) = 0;
  virtual void visit(Reference *) = 0;
  virtual void visit(Array *) = 0;
  virtual void visit(Function *) = 0;
  virtual void visit(PointerToMember *) = 0;
};

}
}

#endif
