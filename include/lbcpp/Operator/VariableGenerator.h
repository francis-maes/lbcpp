/*-----------------------------------------.---------------------------------.
| Filename: VariableGenerator.h            | Base class for Variable         |
| Author  : Francis Maes                   |  Geneators                      |
| Started : 01/02/2011 16:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_VARIABLE_GENERATOR_H_
# define LBCPP_OPERATOR_VARIABLE_GENERATOR_H_

# include "../Core/DynamicObject.h"
# include "../Perception/Perception.h"
# include "Operator.h"

namespace lbcpp
{

class VariableGeneratorCallback
{
public:
  virtual ~VariableGeneratorCallback() {}

  virtual void sense(size_t index, bool value)
    {jassert(false);}

  virtual void sense(size_t index, int value)
    {jassert(false);}

  virtual void sense(size_t index, double value)
    {jassert(false);}

  virtual void sense(size_t index, const String& value)
    {jassert(false);}

  virtual void sense(size_t index, const ObjectPtr& value)
    {jassert(false);}
};

class VariableGenerator : public Operator
{
public:
  virtual void computeVariables(const Variable* inputs, VariableGeneratorCallback& callback) const = 0;

  virtual Variable computeOperator(const Variable* inputs) const;
};

typedef ReferenceCountedObjectPtr<VariableGenerator> VariableGeneratorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_VARIABLE_GENERATOR_H_
