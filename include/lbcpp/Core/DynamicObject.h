/*-----------------------------------------.---------------------------------.
| Filename: DynamicObject.h                | Dynamic Object                  |
| Author  : Francis Maes                   |                                 |
| Started : 26/08/2010 14:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_DYNAMIC_OBJECT_H_
# define LBCPP_CORE_DYNAMIC_OBJECT_H_

# include "Variable.h"

namespace lbcpp
{

class DynamicClass : public DefaultClass
{
public:
  DynamicClass(const String& name, TypePtr baseClass = objectClass)
    : DefaultClass(name, baseClass), variablesType(uncomputedVariableTypes) {}
  DynamicClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)
    : DefaultClass(templateType, templateArguments, baseClass), variablesType(uncomputedVariableTypes) {}
  DynamicClass() : variablesType(uncomputedVariableTypes) {}
  virtual ~DynamicClass();

  ObjectPtr createDenseObject() const;
  ObjectPtr createSparseObject() const;

  /*
  ** Class
  */
  virtual bool initialize(ExecutionContext& context);
  virtual VariableValue create(ExecutionContext& context) const;
  virtual Variable getObjectVariable(const Object* pthis, size_t index) const;
  virtual void setObjectVariable(ExecutionContext& context, Object* pthis, size_t index, const Variable& subValue) const;

  /*
  ** Object
  */
  virtual bool loadFromXml(XmlImporter& importer);

  lbcpp_UseDebuggingNewOperator

protected:
  virtual void createObjectVariables() = 0;

  enum VariablesType
  {
    uncomputedVariableTypes = 0,
    onlyDoubleVariables,
    onlyObjectVariables,
    mixedVariableTypes
  } variablesType;
};

typedef NativePtr<DynamicClass> DynamicClassPtr;
typedef ReferenceCountedObjectPtr<DynamicClass> DynamicClassSharedPtr;

extern ClassPtr enumBasedDoubleVectorClass(TypePtr enumeration);
extern ClassPtr oneSubObjectPerInputVariableClass(TypePtr inputType, TypePtr outputVariablesType);

class UnnamedDynamicClass : public DynamicClass
{
public:
  UnnamedDynamicClass(const String& name, TypePtr baseClass = objectClass)
    : DynamicClass(name, baseClass) {}
  UnnamedDynamicClass() {}

  virtual ClassPtr getClass() const
    {return Object::getClass();}

  virtual bool isUnnamedType() const
    {return true;}

  lbcpp_UseDebuggingNewOperator

protected:
  virtual void createObjectVariables() {}
};

typedef ReferenceCountedObjectPtr<UnnamedDynamicClass> UnnamedDynamicClassPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DYNAMIC_OBJECT_H_
