/*-----------------------------------------.---------------------------------.
| Filename: Variable.hpp                   | A class that encapsulates a     |
| Author  : Francis Maes                   |     C++ variable                |
| Started : 12/03/2009 14:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_STATIC_VARIABLE_HPP_
# define LBCPP_STATIC_VARIABLE_HPP_

# include "../../CRAlgorithm/Variable.h"

namespace lbcpp
{

// todo: make a reference instead of a copy if possible
template<class Type>
class StaticToDynamicVariable : public Variable
{
public:
  typedef StaticToDynamicVariable<Type> ThisClass;
  
  StaticToDynamicVariable(const Type& value, const String& typeName = "", const String& name = "")
    : Variable(new Type(value), typeName, name) {}
  StaticToDynamicVariable()
    : Variable(NULL) {}
    
  virtual ~StaticToDynamicVariable()
    {if (ptr) delete &cast(ptr);}
  
  virtual String toString() const
    {return lbcpp::toString(cast(ptr));}

  virtual bool equals(const VariablePtr otherVariable) const
  {
    const ThisClass* other = dynamic_cast<const ThisClass* >(otherVariable.get());
    return other && cast(ptr) == cast(other->ptr);
  }

private:
  const Type& cast(void* ptr) const
    {return *(const Type* )ptr;}

  Type& cast(void* ptr)
    {return *(Type* )ptr;}
};

template<class T>
inline VariablePtr Variable::create(const T& value, const String& typeName, const String& name)
  {return VariablePtr(new StaticToDynamicVariable<T>(value, typeName, name));}

template<class ContainerType>
class StaticToDynamicVariableIterator : public VariableIterator
{
public:
  typedef Traits<ContainerType> ContainerTraits;
  typedef typename ContainerTraits::ConstIterator iterator;
    
  StaticToDynamicVariableIterator(const ContainerType& container)
    : container(container), it(ContainerTraits::begin(container)) {}
  
  virtual void next()
  {
    if (it != ContainerTraits::end(container))
      ++it;
  }
  
  virtual bool exists() const
    {return it != ContainerTraits::end(container);}
  
  virtual VariablePtr get() const
    {return it == ContainerTraits::end(container) 
      ? VariablePtr() 
      : Variable::create(ContainerTraits::value(it));}

private:
  const ContainerType& container;
  iterator it;
};

template<class T>
inline VariableIteratorPtr VariableIterator::create(const T& container)
  {return VariableIteratorPtr(new StaticToDynamicVariableIterator<T>(container));}

}; /* namespace lbcpp */

#endif // !LBCPP_STATIC_VARIABLE_HPP_
