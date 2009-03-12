/*-----------------------------------------.---------------------------------.
| Filename: Variable.hpp                   | A class that encapsulates a     |
| Author  : Francis Maes                   |     C++ variable                |
| Started : 12/03/2009 14:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_STATIC_VARIABLE_HPP_
# define CRALGO_STATIC_VARIABLE_HPP_

# include "../../Variable.h"

namespace cralgo
{

// todo: make a reference instead of a copy if possible
template<class ChoiceType>
class StaticToDynamicVariable : public Variable
{
public:
  StaticToDynamicVariable(const ChoiceType& choice, const std::string& typeName = "", const std::string& name = "")
    : Variable(new ChoiceType(choice), typeName, name) {}
    
  virtual ~StaticToDynamicVariable()
    {delete &cast(ptr);}
  
  virtual std::string toString() const
    {return cralgo::toString(cast(ptr));}

private:
  const ChoiceType& cast(void* ptr) const
    {return *(const ChoiceType* )ptr;}

  ChoiceType& cast(void* ptr)
    {return *(ChoiceType* )ptr;}
};

template<class T>
inline VariablePtr Variable::create(const T& value, const std::string& typeName, const std::string& name)
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

}; /* namespace cralgo */

#endif // !CRALGO_STATIC_VARIABLE_HPP_
