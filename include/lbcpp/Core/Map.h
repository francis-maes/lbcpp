/*-----------------------------------------.---------------------------------.
| Filename: Map.h                          | Map Containeer base class       |
| Author  : Arnaud Schoofs                 | TODO arnaud : not finished !    |
| Started : 10/03/2011 21:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_MAP_H_
# define LBCPP_CORE_MAP_H_

# include "Container.h"

namespace lbcpp
{
  
class Map : public Object
{
public:
  Map(ClassPtr thisClass) : Object(thisClass) {}
  Map() {}

  static TypePtr getTemplateKeysParameter(TypePtr type);
  static bool getTemplateKeysParameter(ExecutionContext& context, TypePtr type, TypePtr& res);
  static TypePtr getTemplateValuesParameter(TypePtr type);
  static bool getTemplateValuesParameter(ExecutionContext& context, TypePtr type, TypePtr& res);
  
  virtual TypePtr getKeysType() const
    {jassert(thisClass); return thisClass->getTemplateArgument(0);}
  
  virtual TypePtr getValuesType() const
    {jassert(thisClass); return thisClass->getTemplateArgument(1);}  
  
  //TypePtr computeKeysCommonBaseType() const;

  bool isEmpty() const
    {return getNumElements() == 0;}
  
  
  virtual size_t getNumElements() const = 0;
  
  //int findElement(const Variable& value) const;
  
    
  //virtual EnumerationPtr getElementsEnumeration() const
  //{return positiveIntegerEnumerationEnumeration;}
  
  //String getElementName(size_t index) const;
  
  //virtual Variable getElement(size_t index) const = 0;
  //virtual void setElement(size_t index, const Variable& value) = 0;
  
  //virtual String toString() const;
  //virtual String toShortString() const;
  //VectorPtr toVector() const;
  
  //virtual void saveToXml(XmlExporter& exporter) const;
  //virtual bool loadFromXml(XmlImporter& importer);
  
  //virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;
  
  //void makeOrder(size_t variableIndex, bool increasingOrder, std::vector<size_t>& res) const;
  
  lbcpp_UseDebuggingNewOperator
  
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_MAP_H_
