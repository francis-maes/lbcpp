/*-----------------------------------------.---------------------------------.
| Filename: DenseDoubleObject.h            | Dense Double Object             |
| Author  : Francis Maes                   |                                 |
| Started : 08/10/2010 16:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_OBJECT_DENSE_DOUBLE_H_
# define LBCPP_DATA_OBJECT_DENSE_DOUBLE_H_

# include <lbcpp/Data/DynamicObject.h>
# include <lbcpp/Data/XmlSerialisation.h>

namespace lbcpp
{

class DenseDoubleObject : public Object
{
public:
  DenseDoubleObject(TypePtr thisType)
    : Object(thisType)
  {
    missingValue = doubleType->getMissingValue().getDouble();
  }

  double& getValueReference(size_t index)
  {
    jassert(index < thisClass->getObjectNumVariables());
    if (values.size() <= index)
    {
      size_t i = values.size();
      values.resize(index + 1, missingValue);
    }
    return values[index];
  }

  bool isMissing(double value) const
    {return value == missingValue;}

  virtual Variable getVariable(size_t index) const
  {
    TypePtr type = thisClass->getObjectVariableType(index);
    if (index < values.size() && values[index] != missingValue)
      return Variable(values[index], type);
    else
      return Variable::missingValue(type);
  }

  virtual void setVariable(size_t index, const Variable& value)
    {getValueReference(index) = value.getDouble();}

  virtual VariableIterator* createVariablesIterator() const;

  virtual String toString() const
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

  virtual void saveToXml(XmlExporter& exporter) const
    {exporter.addTextElement(toString());}

  virtual bool loadFromString(const String& str, MessageCallback& callback)
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

  virtual bool loadFromXml(XmlImporter& importer)
    {return loadFromString(importer.getAllSubText(), importer.getCallback());}

private:
  friend class DenseDoubleObjectVariableIterator;

  std::vector<double> values;
  double missingValue;
};

typedef ReferenceCountedObjectPtr<DenseDoubleObject> DenseDoubleObjectPtr;

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
      TypePtr type = object->thisClass->getObjectVariableType(current);
      if (object->values[current] != object->missingValue)
      {
        currentValue = Variable(object->values[current], type);
        break;
      }
      ++current;
    }
  }
};

inline Object::VariableIterator* DenseDoubleObject::createVariablesIterator() const
  {return new DenseDoubleObjectVariableIterator(refCountedPointerFromThis(this));}

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_OBJECT_DENSE_DOUBLE_H_
