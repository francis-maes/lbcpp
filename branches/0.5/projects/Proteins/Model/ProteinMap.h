/*-----------------------------------------.---------------------------------.
| Filename: ProteinMap.h                   |                                 |
| Author  : Julien Becker                  |                                 |
| Started : 12/02/2012 13:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef _PROTEINS_MAP_H_
# define _PROTEINS_MAP_H_

# include "../Data/Protein.h"

namespace lbcpp
{

class Map : public Object
{
public:
  bool isExists(const String& key) const
    {return map.count(key) == 1;}

  Variable getValue(const String& key) const
    {jassert(isExists(key)); return map.at(key);}

  void setValue(const String& key, const Variable& value)
    {map[key] = value;}

protected:
  std::map<String, Variable> map;
};

class ProteinMap : public Map
{
public:
  ProteinMap(ProteinPtr protein) : protein(protein)
  {
    jassert(protein);
    length = protein->getLength();
  }

  ProteinPtr getProtein() const
    {return protein;}

  Variable getValueOrComputeIfMissing(ExecutionContext& context,
                                      const String& variableName,
                                      const FunctionPtr& function)
  {
    if (isExists(variableName))
      return getValue(variableName);
    Variable res = function->compute(context, refCountedPointerFromThis(this));
    setValue(variableName, res);
    return res;
  }
  
protected:
  friend class ProteinMapClass;

  ProteinPtr protein;
  size_t length;

  ProteinMap() {}
};

typedef ReferenceCountedObjectPtr<ProteinMap> ProteinMapPtr;

extern ClassPtr proteinMapClass;

class CreateProteinMap : public SimpleUnaryFunction
{
public:
  CreateProteinMap()
    : SimpleUnaryFunction(proteinClass, proteinMapClass, T("Map")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return new ProteinMap(input.dynamicCast<Protein>());}
};

class GetProteinMapElement : public Function
{
public:
  GetProteinMapElement(const String& elementName)
    : variableName(elementName) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return proteinMapClass;}

  virtual String getOutputPostFix() const
    {return T("Member");}
  
  virtual TypePtr initializeFunction(ExecutionContext& context,
                                     const std::vector<VariableSignaturePtr>& inputVariables,
                                     String& outputName, String& outputShortName)
  {
    if (!function->initialize(context, proteinMapClass))
    {
      context.errorCallback(T("GetProteinMapElement::initializeFunction"),
                            T("Initialization error with the function: ") +
                            function->getClassName());
      return TypePtr();
    }
    return function->getOutputType();
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    ProteinMapPtr map = input.getObjectAndCast<ProteinMap>();
    jassert(map);
    return map->getValueOrComputeIfMissing(context, variableName, function);
  }
  
protected:
  friend class GetProteinMapElementClass;

  GetProteinMapElement() {}

  String variableName;
  FunctionPtr function;
};

}; /* namespace lbcpp */

#endif // _PROTEINS_MAP_H_
