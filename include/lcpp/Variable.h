/*-----------------------------------------.---------------------------------.
| Filename: Variable.h                     | A class that encapsulates a     |
| Author  : Francis Maes                   |     C++ variable                |
| Started : 12/03/2009 14:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LCPP_VARIABLE_H_
# define LCPP_VARIABLE_H_

# include "ObjectPredeclarations.h"
# include "ChooseFunction.h"

namespace lcpp
{

class Variable : public Object
{
public:
  Variable(void* ptr, const std::string& typeName = "", const std::string& name = "")
    : ptr(ptr), typeName(typeName), name(name) {}
    
  template<class T>
  inline T& getReference()
    {return *(T* )ptr;}

  template<class T>
  inline const T& getConstReference() const
    {return *(const T* )ptr;}

  template<class T>
  inline T getCopy() const
    {return *(const T* )ptr;}
  
  inline const void* getUntypedPointer() const
    {return this ? ptr : NULL;}

  inline void*& getUntypedPointer()
    {return ptr;}
    
  template<class T>
  static VariablePtr create(const T& value, const std::string& typeName = "", const std::string& name = "");

  static VariablePtr createFromPointer(void* value, const std::string& typeName = "", const std::string& name = "")
    {return VariablePtr(new Variable(value, typeName, name));}

  template<class T>
  static VariablePtr createFromPointer(T* value, const std::string& typeName = "", const std::string& name = "")
    {return create(*value, typeName, name);}

  // Object
  virtual std::string getName() const
    {return name;}
    
  virtual bool equals(const VariablePtr otherVariable) const
    {assert(false); return false;}
    
  std::string getTypeName() const
    {return typeName;}
    
protected:
  void* ptr;
  std::string typeName;
  std::string name;
};

template<>
struct Traits<VariablePtr> : public ObjectPtrTraits<Variable> {};

class VariableIterator : public ReferenceCountedObject
{
public:
  template<class T>
  static VariableIteratorPtr create(const T& value);

  virtual bool exists() const = 0;
  virtual VariablePtr get() const = 0;
  virtual void next() = 0;
};

}; /* namespace lcpp */

#endif // !LCPP_VARIABLE_H_
