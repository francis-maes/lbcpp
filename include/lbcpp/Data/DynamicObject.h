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

class DynamicClass : public DefaultClass
{
public:
  DynamicClass(const String& name, TypePtr baseClass = objectClass)
    : DefaultClass(name, baseClass) {}
  DynamicClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)
    : DefaultClass(templateType, templateArguments, baseClass) {}
  DynamicClass() {}
  virtual ~DynamicClass();

  ObjectPtr createDenseObject() const;
  ObjectPtr createSparseObject() const;

  /*
  ** Class
  */
  virtual bool initialize(MessageCallback& callback);
  virtual VariableValue create() const;
  virtual Variable getObjectVariable(const Object* pthis, size_t index) const;
  virtual void setObjectVariable(Object* pthis, size_t index, const Variable& subValue) const;

  /*
  ** Object
  */
  virtual ClassPtr getClass() const
    {return Object::getClass();}

  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

protected:
  virtual void createObjectVariables() {}
};

typedef ReferenceCountedObjectPtr<DynamicClass> DynamicClassPtr;

extern ClassPtr enumBasedDoubleVectorClass(TypePtr enumeration);
extern ClassPtr oneSubObjectPerInputVariableClass(TypePtr inputType, TypePtr outputVariablesType);

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_DYNAMIC_OBJECT_H_
