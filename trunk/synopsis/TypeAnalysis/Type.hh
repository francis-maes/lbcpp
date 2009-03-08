//
// Copyright (C) 2005 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_TypeAnalysis_Type_hh_
#define Synopsis_TypeAnalysis_Type_hh_

#include <Synopsis/TypeAnalysis/Visitor.hh>
#include <string>
#include <ostream>
#include <iterator>

namespace Synopsis
{
namespace TypeAnalysis
{

class Type
{
public:
  Type(std::string const &name) : my_name(name), my_refcounter(1) {}
  virtual ~Type() {}
  const std::string &name() const { return my_name;}
  virtual void accept(Visitor *visitor) = 0;
  virtual void ref() const { ++my_refcounter;}
  virtual void deref() const { if (--my_refcounter) delete this;}
  // TODO: add 'new' / 'delete' operators for optimization.
private:
  const std::string my_name;
  mutable size_t    my_refcounter;
};

class BuiltinType : public Type
{
public:
  BuiltinType(std::string const &name) : Type(name) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
  // BuiltinType is preallocated and thus is destructed at program termination.
  virtual void ref() const {}
  virtual void deref() const {}
};

extern BuiltinType BOOL;
extern BuiltinType CHAR;
extern BuiltinType WCHAR;
extern BuiltinType SHORT;
extern BuiltinType INT;
extern BuiltinType LONG;
extern BuiltinType FLOAT;
extern BuiltinType DOUBLE;
extern BuiltinType UCHAR;
extern BuiltinType USHORT;
extern BuiltinType UINT;
extern BuiltinType ULONG;
extern BuiltinType SCHAR;
extern BuiltinType SSHORT;
extern BuiltinType SINT;
extern BuiltinType SLONG;

class Enum : public Type
{
public:
  Enum(std::string const &name) : Type(name) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class Compound : public Type
{
public:
  Compound(std::string const &name) : Type(name) {}
};

class Class : public Compound
{
public:
  enum Kind { STRUCT, CLASS};

  Class(Kind kind, std::string const &name) : Compound(name), my_kind(kind) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}

private:
  Kind my_kind;
};

class Union : public Compound
{
public:
  Union(std::string const &name) : Compound(name) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}

private:
};

class CVType : public Type
{
public:
  enum CVQualifier { NONE=0x0, CONST=0x1, VOLATILE=0x2};

  CVType(Type const *type, CVQualifier q)
    : Type(names[q]), my_type(type), my_qual(q) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}

private:
  static std::string const names[4];

  Type const *my_type;
  CVQualifier my_qual;
};

class Pointer : public Type
{
public:
  Pointer(Type const *type) : Type("*"), my_type(type) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}

private:
  Type const *my_type;
};

class Reference : public Type
{
public:
  Reference(Type const *type) : Type("&"), my_type(type) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}

private:
  Type const *my_type;
};

class Array : public Type
{
public:
  Array(Type const *type) : Type("[]"), my_type(type) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}

private:
  Type const *my_type;
};

class Function : public Type
{
public:
  Function() : Type("") {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}

private:
  Type const *my_type;
};

class PointerToMember : public Type
{
public:
  PointerToMember() : Type("") {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}

private:
  Type const *my_container;
  Type const *my_member;
};

}
}

#endif
