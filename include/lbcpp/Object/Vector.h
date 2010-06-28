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
  Vector(ClassPtr type = topLevelClass()) : type(type) {}

  virtual ~Vector()
    {clear();}

  ClassPtr getType() const
    {return type;}

  virtual size_t size() const;
  virtual Variable getVariable(size_t index) const;
  virtual void setVariable(size_t index, const Variable& value);

  void reserve(size_t size)
    {values.reserve(size);}

  void clear();
  void append(const Variable& value);

private:
  ClassPtr type;
  std::vector<VariableValue> values;

  bool checkType(const Variable& value) const;
};

typedef ReferenceCountedObjectPtr<Vector> VectorPtr;

extern ClassPtr vectorClass(ClassPtr elementsClass);

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_VECTOR_H_
