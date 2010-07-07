/*-----------------------------------------.---------------------------------.
| Filename: Vector.h                       | Vector of variables             |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_VECTOR_H_
# define LBCPP_OBJECT_VECTOR_H_

# include "VariableContainer.h"

namespace lbcpp
{

class Vector : public VariableContainer
{
public:
  Vector() {}

  Vector(TypePtr type, size_t initialSize = 0) : type(type)
  {
    jassert(type != topLevelType());
    if (initialSize)
      values.resize(initialSize, VariableValue());
  }

  virtual ~Vector()
    {clear();}

  virtual TypePtr getClass() const;

  virtual TypePtr getStaticType() const
    {return type;}

  virtual size_t getNumVariables() const;
  virtual Variable getVariable(size_t index) const;
  virtual void setVariable(size_t index, const Variable& value);

  void reserve(size_t size)
    {values.reserve(size);}

  void clear();
  void append(const Variable& value);

private:
  TypePtr type;
  std::vector<VariableValue> values;
  
  bool checkType(const Variable& value) const;
};

typedef ReferenceCountedObjectPtr<Vector> VectorPtr;

extern ClassPtr vectorClass(TypePtr elementsType);

class BooleanVector : public VariableContainer
{
public:
  BooleanVector(size_t initialSize = 0)
    : v(initialSize, false) {}

  virtual String toString() const
  {
    String res = T("[");
    for (size_t i = 0; i < v.size(); ++i)
      res += v[i] ? '+' : '-';
    res += T("]");
    return res;
  }

  virtual TypePtr getStaticType() const
    {return booleanType();}

  virtual size_t getNumVariables() const
    {return v.size();}

  virtual Variable getVariable(size_t index) const
    {jassert(index < v.size()); return v[index];}

  virtual void setVariable(size_t index, const Variable& value)
  {
    if (checkInheritance(value, booleanType()))
      v[index] = value.getBoolean();
  }

  void set(size_t index, bool value)
    {jassert(index < v.size()); v[index] = value;}

  bool get(size_t index) const
    {jassert(index < v.size()); return v[index];}

protected:
  std::vector<bool> v;
};

typedef ReferenceCountedObjectPtr<BooleanVector> BooleanVectorPtr;

class DynamicTypeVector : public VariableContainer
{
public:
  DynamicTypeVector() {}

  virtual size_t getNumVariables() const
    {return variables.size();}

  virtual Variable getVariable(size_t index) const
    {jassert(index < variables.size()); return variables[index];}

  virtual void setVariable(size_t index, const Variable& value)
    {jassert(index < variables.size()); variables[index] = value;}

  void reserve(size_t size)
    {variables.reserve(size);}

  void clear()
    {variables.clear();}

  void append(const Variable& value)
    {variables.push_back(value);}

private:
  std::vector<Variable> variables;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_VECTOR_H_
