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

class DenseObjectObject : public Object
{
public:
  DenseObjectObject(DynamicClassSharedPtr thisClass);

  ObjectPtr& getObjectReference(size_t index);

  size_t getNumObjects() const
    {return values.size();}

  const ObjectPtr& getObject(size_t index) const
    {static ObjectPtr empty; return index < values.size() ? values[index] : empty;}

  virtual Variable getVariable(size_t index) const;
  virtual void setVariable(ExecutionContext& context, size_t index, const Variable& value);
  virtual VariableIterator* createVariablesIterator() const;

  virtual String toString() const;

private:
  friend class DenseObjectObjectVariableIterator;

  std::vector<ObjectPtr> values;
  DynamicClassSharedPtr thisClass;
};

class DenseDoubleObject : public Object
{
public:
  DenseDoubleObject(DynamicClassSharedPtr thisClass);
  DenseDoubleObject(DynamicClassSharedPtr thisClass, double initialValue);

  double& getValueReference(size_t index);

  double getValue(size_t index) const
    {return index < values.size() ? values[index] : missingValue;}

  const std::vector<double>& getValues() const
    {return values;}

  std::vector<double>& getValues()
    {return values;}

  bool isMissing(double value) const
    {return value == missingValue;}

  virtual Variable getVariable(size_t index) const;
  virtual void setVariable(ExecutionContext& context, size_t index, const Variable& value);
  virtual VariableIterator* createVariablesIterator() const;

  virtual String toString() const;
  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromString(ExecutionContext& context, const String& str);
  virtual bool loadFromXml(XmlImporter& importer);

  DenseDoubleObjectPtr createCompatibleNullObject() const
    {return new DenseDoubleObject(thisClass, 0.0);}

private:
  friend class DenseDoubleObjectVariableIterator;
  
  DynamicClassSharedPtr thisClass;
  std::vector<double> values;
  double missingValue;
};

class SparseDoubleObject : public Object
{
public:
  SparseDoubleObject(DynamicClassSharedPtr thisClass);
  
  virtual String toShortString() const;

  virtual Variable getVariable(size_t index) const;
  virtual void setVariable(ExecutionContext& context, size_t index, const Variable& value);

  VariableIterator* createVariablesIterator() const;

  std::vector< std::pair<size_t, double> >& getValues()
    {return values;}

  const std::vector< std::pair<size_t, double> >& getValues() const
    {return values;}

  void reserveValues(size_t count)
    {values.reserve(count);}

  void appendValue(size_t index, double value);

private:
  friend class SparseDoubleObjectVariableIterator;

  DynamicClassSharedPtr thisClass;
  std::vector< std::pair<size_t, double> > values;
  int lastIndex;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DYNAMIC_OBJECT_H_
