/*-----------------------------------------.---------------------------------.
| Filename: DynamicObject.h                | Dynamic Object                  |
| Author  : Francis Maes                   |                                 |
| Started : 26/08/2010 14:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_DYNAMIC_OBJECT_H_
# define LBCPP_DATA_DYNAMIC_OBJECT_H_

# include "Variable.h"

namespace lbcpp
{

class DynamicObject : public Object
{
public:
  DynamicObject(TypePtr thisType);
  virtual ~DynamicObject();

  VariableValue& operator[](size_t index);

private:
  std::vector<VariableValue> variableValues;
};

typedef ReferenceCountedObjectPtr<DynamicObject> DynamicObjectPtr;

extern ClassPtr dynamicClassClass();

class DynamicClass : public DefaultClass
{
public:
  DynamicClass(const String& name, TypePtr baseClass = objectClass())
    : DefaultClass(name, baseClass) {}
  DynamicClass() {}

  /*
  ** Class
  */
  virtual VariableValue create() const;
  virtual Variable getObjectVariable(const VariableValue& value, size_t index) const;
  virtual void setObjectVariable(const VariableValue& value, size_t index, const Variable& subValue) const;

  /*
  ** Object
  */
  virtual ClassPtr getClass() const
    {return Object::getClass();}

  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlElement* xml, MessageCallback& callback);
};

typedef ReferenceCountedObjectPtr<DynamicClass> DynamicClassPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_DYNAMIC_OBJECT_H_
