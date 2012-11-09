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
  ** DefaultClass
  */
  size_t addMemberVariable(ExecutionContext& context, TypePtr type, const String& name, const String& shortName = String::empty, const String& description = String::empty)
    {return DefaultClass::addMemberVariable(context, type, name, shortName, description);}

  size_t addMemberVariable(ExecutionContext& context, const String& typeName, const String& name, const String& shortName = String::empty, const String& description = String::empty)
    {return DefaultClass::addMemberVariable(context, typeName, name, shortName, description);}

  virtual size_t addMemberVariable(ExecutionContext& context, VariableSignaturePtr signature)
    {ScopedLock _(lock); return DefaultClass::addMemberVariable(context, signature);}

  /*
  ** Class
  */
  virtual bool initialize(ExecutionContext& context);
  virtual ObjectPtr create(ExecutionContext& context) const
    {return createDenseObject();}
  virtual Variable getMemberVariableValue(const Object* pthis, size_t index) const;
  virtual void setMemberVariableValue(Object* pthis, size_t index, const Variable& subValue) const;

  /*
  ** Object
  */
  virtual bool loadFromXml(XmlImporter& importer);

  CriticalSection lock;

  lbcpp_UseDebuggingNewOperator

protected:
  virtual void createObjectVariables() {}

  enum VariablesType
  {
    uncomputedVariableTypes = 0,
    onlyDoubleVariables,
    onlyObjectVariables,
    mixedVariableTypes
  } variablesType;

  void ensureVariablesTypeIsComputed();
  void computeVariablesType();
};

extern ClassPtr enumBasedDoubleVectorClass(TypePtr enumeration, TypePtr variablesType = doubleType);
extern ClassPtr oneSubObjectPerInputVariableClass(TypePtr inputType, TypePtr outputVariablesType);

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DYNAMIC_OBJECT_H_
