/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmScope.h             | CR-algorithm scope base class   |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 18:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_CRALGORITHM_SCOPE_H_
# define CRALGO_CRALGORITHM_SCOPE_H_

# include "Variable.h"

namespace cralgo
{

class CRAlgorithmScope : public Object
{
public:
  /*
  ** Introspection
  */
  virtual size_t getNumVariables() const = 0;
  virtual VariablePtr getVariable(size_t num) const = 0;
  virtual VariablePtr getVariable(const std::string& name) const = 0;

  template<class T>
  const T& getVariableReference(const std::string& name) const
  {
    VariablePtr v = getVariable(name);
    if (!v)
    {
      Object::error("CRAlgorithmScope::getVariableReference", "Could not find variable called '" + name + "'");
      assert(false);
    }
    return v->getConstReference<T>();
  }
  
  virtual std::string getVariableType(size_t num) const = 0;
  virtual std::string getVariableName(size_t num) const = 0;
  virtual std::string getVariableValue(size_t num) const = 0;
  
  /*
  ** Clone / assignment / swap
  */
  virtual CRAlgorithmScopePtr cloneScope() const = 0;
  virtual void setScope(const CRAlgorithmScope& otherScope) = 0;
  virtual void swapScope(CRAlgorithmScope& otherScope) = 0;

  /*
  ** Current State
  */
  virtual int getState() const = 0;
  virtual CRAlgorithmScopePtr getCurrentInnerScope() = 0;
  
  /*
  ** To string
  */
  virtual std::string toString() const
  {
    std::string res;
    for (size_t i = 0; i < getNumVariables(); ++i)
      res += getVariableName(i) + " = " + getVariableValue(i) + "\n";
    return res;
  }  
};

}; /* namespace cralgo */

#endif // !CRALGO_CRALGORITHM_SCOPE_H_
