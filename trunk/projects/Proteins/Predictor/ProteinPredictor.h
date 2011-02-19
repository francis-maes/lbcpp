/*-----------------------------------------.---------------------------------.
| Filename: ProteinPredictor.h             | Protein Predictor               |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 14:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PREDICTOR_H_
# define LBCPP_PROTEIN_PREDICTOR_H_

# include "ProteinPredictorParameters.h"

namespace lbcpp
{

// protein, protein -> protein
class ProteinPredictor : public CompositeFunction
{
public:
  ProteinPredictor(ProteinPredictorParametersPtr factory);
  ProteinPredictor() {}

  void addTarget(ProteinTarget target);

  virtual void buildFunction(CompositeFunctionBuilder& builder);

protected:
  ProteinPredictorParametersPtr factory;
  std::vector< std::pair<ProteinTarget, FunctionPtr> > targetPredictors;
};

typedef ReferenceCountedObjectPtr<ProteinPredictor> ProteinPredictorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PREDICTOR_H_
