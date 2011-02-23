/*-----------------------------------------.---------------------------------.
| Filename: ProteinEvaluator.h             | Protein Evaluator               |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 23:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_EVALUATOR_H_
# define LBCPP_PROTEIN_EVALUATOR_H_

# include <lbcpp/Core/CompositeFunction.h>
# include "ContactMapEvaluator.h"
# include "TertiaryStructureEvaluator.h"

namespace lbcpp
{

class MergeContainerFunction : public SimpleUnaryFunction
{
public:
  MergeContainerFunction()
    : SimpleUnaryFunction(containerClass(containerClass(anyType)), containerClass(anyType))
    {}
  
protected:
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    ContainerPtr inputContainer = input.getObjectAndCast<Container>();
    VectorPtr res = vector(inputContainer->getElementsType()->getTemplateArgument(0));
    size_t n = inputContainer->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ContainerPtr values = inputContainer->getElement(i).getObjectAndCast<Container>();
      const size_t numElements = values->getNumElements();
      const size_t totalNumElements = res->getNumElements();
      res->resize(totalNumElements + numElements);
      for (size_t i = 0; i < numElements; ++i)
        res->setElement(totalNumElements + i, values->getElement(i));
    }
    return res;
  }
};

class ProteinEvaluatorCompositeFunction : public CompositeFunction
{
public:
  ProteinEvaluatorCompositeFunction()
    {numInputs = 2;}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t predicted = builder.addInput(containerClass(proteinClass), T("predicted"));
    size_t supervision = builder.addInput(containerClass(proteinClass), T("supervision"));
    
    size_t ss3 = builder.addFunction(mapContainerFunction(getVariableFunction(ss3Target)), predicted, T("ss3"));
    ss3 = builder.addFunction(new MergeContainerFunction(), ss3, T("ss3"));
    
    size_t ss3Supervision = builder.addFunction(mapContainerFunction(getVariableFunction(ss3Target)), supervision, T("ss3Supervision"));
    ss3Supervision = builder.addFunction(new MergeContainerFunction(), ss3Supervision, T("ss3Supervision"));
    
    builder.startSelection();

      builder.addFunction(classificationEvaluator(), ss3, ss3Supervision, T("ss3"));

    builder.finishSelectionWithFunction(concatenateScoreObjectFunction());
  }

};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_H_
