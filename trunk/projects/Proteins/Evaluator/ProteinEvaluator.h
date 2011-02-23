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

class ProteinEvaluatorCompositeFunction : public CompositeFunction
{
public:
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t predicted = builder.addInput(containerClass(proteinClass), T("predicted"));
    size_t supervision = builder.addInput(containerClass(proteinClass), T("supervision"));
    
    size_t ss3 = builder.addFunction(mapContainerFunction(getVariableFunction(ss3Target)), predicted, T("ss3"));
    ss3 = builder.addFunction(concatenateContainerFunction(), ss3, T("ss3"));
    
    size_t ss3Supervision = builder.addFunction(mapContainerFunction(getVariableFunction(ss3Target)), supervision, T("ss3Supervision"));
    ss3Supervision = builder.addFunction(concatenateContainerFunction(), ss3Supervision, T("ss3Supervision"));
    
    builder.startSelection();

      builder.addFunction(classificationEvaluator(), ss3, ss3Supervision, T("ss3"));

    builder.finishSelectionWithFunction(concatenateScoreObjectFunction());
  }

};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_H_
