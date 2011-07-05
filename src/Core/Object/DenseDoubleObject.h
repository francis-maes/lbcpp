/*-----------------------------------------.---------------------------------.
| Filename: DenseDoubleObject.h            | Dense Double Object             |
| Author  : Francis Maes                   |                                 |
| Started : 08/10/2010 16:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_OBJECT_DENSE_DOUBLE_H_
# define LBCPP_DATA_OBJECT_DENSE_DOUBLE_H_

# include <lbcpp/Core/DynamicObject.h>
# include <lbcpp/Core/XmlSerialisation.h>

namespace lbcpp
{
  
class DenseDoubleObjectVariableIterator : public Object::VariableIterator
{
public:
  DenseDoubleObjectVariableIterator(DenseDoubleObjectPtr object)
    : object(object), current(0), n(object->values.size()) {moveToNextActiveVariable();}

  virtual bool exists() const
    {return current < n;}
  
  virtual Variable getCurrentVariable(size_t& index) const
    {jassert(current < n); index = current; return currentValue;}

  virtual void next()
  {
    jassert(current < n);
    ++current;
    moveToNextActiveVariable();
  }

private:
  DenseDoubleObjectPtr object;
  size_t current;
  Variable currentValue;
  size_t n;

  void moveToNextActiveVariable()
  {
    while (current < n)
    {
      TypePtr type = object->thisClass->getMemberVariableType(current);
      if (object->values[current] != object->missingValue)
      {
        currentValue = Variable(object->values[current], type);
        break;
      }
      ++current;
    }
  }
};

DenseDoubleObject::DenseDoubleObject(DynamicClassPtr thisClass)
  : Object(thisClass)
{
  missingValue = doubleType->getMissingValue().getDouble();
}

DenseDoubleObject::DenseDoubleObject(DynamicClassPtr thisClass, double initialValue)
  : Object(thisClass), values(thisClass->getNumMemberVariables(), initialValue)
{
  missingValue = doubleType->getMissingValue().getDouble();
}

DenseDoubleObject::DenseDoubleObject(DynamicClassPtr thisClass, const std::vector<double>& values)
  : Object(thisClass), values(values)
{
  missingValue = doubleType->getMissingValue().getDouble();
}

double& DenseDoubleObject::getValueReference(size_t index)
{
  jassert(index < thisClass->getNumMemberVariables());
  if (values.size() <= index)
    values.resize(index + 1, missingValue);
  return values[index];
}

Variable DenseDoubleObject::getVariable(size_t index) const
{
  TypePtr type = thisClass->getMemberVariableType(index);
  if (index < values.size() && values[index] != missingValue)
    return Variable(values[index], type);
  else
    return Variable::missingValue(type);
}

void DenseDoubleObject::setVariable(size_t index, const Variable& value)
  {getValueReference(index) = value.toDouble();}

String DenseDoubleObject::toString() const
{
  size_t n = getNumVariables();
  String res;
  for (size_t i = 0; i < n; ++i)
  {
    if (i >= values.size() || values[i] == missingValue)
      res += T("_");
    else
      res += String(values[i]);
    if (i < n - 1)
      res += T(" ");
  }
  return res;
}

void DenseDoubleObject::saveToXml(XmlExporter& exporter) const
  {exporter.addTextElement(toString());}

bool DenseDoubleObject::loadFromString(ExecutionContext& context, const String& str)
{
  StringArray tokens;
  tokens.addTokens(str, false);
  values.resize(tokens.size());
  for (int i = 0; i < tokens.size(); ++i)
  {
    String str = tokens[i];
    if (str == T("_"))
      values[i] = missingValue;
    else
      values[i] = str.getDoubleValue();
  }
  return true;
}

bool DenseDoubleObject::loadFromXml(XmlImporter& importer)
  {return loadFromString(importer.getContext(), importer.getAllSubText());}

Object::VariableIterator* DenseDoubleObject::createVariablesIterator() const
  {return new DenseDoubleObjectVariableIterator(refCountedPointerFromThis(this));}

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_OBJECT_DENSE_DOUBLE_H_
