/*-----------------------------------------.---------------------------------.
| Filename: Vector.cpp                     | Vector of variables             |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Object/Vector.h>
using namespace lbcpp;

size_t Vector::getNumVariables() const
  {return values.size();}

Variable Vector::getVariable(size_t index) const
  {jassert(index < values.size()); return Variable::copyFrom(type, values[index]);}

void Vector::setVariable(size_t index, const Variable& value)
{
  if (checkType(value))
  {
    jassert(index < values.size());
    value.copyTo(values[index]);
  }
}

void Vector::clear()
{
  for (size_t i = 0; i < values.size(); ++i)
    type->destroy(values[i]);
  values.clear();
}

void Vector::append(const Variable& value)
{
  if (checkType(value))
  {
    values.push_back(VariableValue());
    value.copyTo(values.back());
  }
}

bool Vector::checkType(const Variable& value) const
{
  if (!value.getType()->inheritsFrom(type))
  {
    Object::error(T("Vector::checkType"), T("Invalid type, Expected ") + type->getName() + T(" found ") + value.getTypeName());
    return false;
  }
  return true;
}

ClassPtr lbcpp::vectorClass(TypePtr elementsType)
{
  return Class::get(T("Vector"));
}


// todo: ranger
#include <lbcpp/Object/SymmetricMatrix.h>
ClassPtr lbcpp::symmetricMatrixClass(TypePtr elementsType)
  {return Class::get(T("SymmetricMatrix"));}
