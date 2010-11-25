/*-----------------------------------------.---------------------------------.
| Filename: ContactMapInference.h          | Contact Map Inference           |
| Author  : Francis Maes                   |                                 |
| Started : 15/07/2010 16:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_H_
# define LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_H_

# include <lbcpp/Data/SymmetricMatrix.h>
# include <lbcpp/Inference/ParallelInference.h>
# include "../Data/Protein.h"
# include "DisulfideBondsInference.h"

namespace lbcpp
{

class ContactMapInference : public SymmetricProbabilityMatrixInference
{
public:
  ContactMapInference(const String& name, InferencePtr contactInference)
    : SymmetricProbabilityMatrixInference(name, contactInference) {}
  ContactMapInference() {}

  virtual TypePtr getInputType() const
    {return proteinClass;}

  virtual ParallelInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const ProteinPtr& inputProtein = input.getObjectAndCast<Protein>(context);
    const SymmetricMatrixPtr& supervisionMap = supervision.getObjectAndCast<SymmetricMatrix>(context);
    jassert(inputProtein && (!supervision.exists() || supervisionMap));

    size_t n = inputProtein->getLength();

    ParallelInferenceStatePtr res = new State(input, supervision, n, 6);
    res->reserve((n * (n - 6)) / 2);
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 6; j < n; ++j)
      {
        Variable elementSupervision;
        if (supervisionMap)
          elementSupervision = supervisionMap->getElement(i, j);
        res->addSubInference(subInference, Variable::pair(input, Variable::pair(i, j)), elementSupervision);
      }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_H_

