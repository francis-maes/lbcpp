/*-----------------------------------------.---------------------------------.
| Filename: TertiaryStructureEvaluator.h   | Tertiary Structure Evaluator    |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 23:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_EVALUATOR_TERTIARY_STRUCTURE_H_
# define LBCPP_PROTEIN_EVALUATOR_TERTIARY_STRUCTURE_H_

# include <lbcpp/Function/Evaluator.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class TertiaryStructureEvaluator : public Evaluator
{
public:
  TertiaryStructureEvaluator(const String& name)
    : Evaluator(name), calphaRMSE(new ScalarVariableMean()) {}

  virtual String toString() const
  {
    double count = calphaRMSE->getCount();
    if (!count)
      return String::empty;
    return T("C-Alpha RMSE = ") + String(calphaRMSE->getMean(), 4);
  }

  virtual double getDefaultScore() const
    {return -calphaRMSE->getMean();}
 
  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {res.push_back(std::make_pair(T("C-Alpha RMSE"), -calphaRMSE->getMean()));}
 
  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject)
  {
    if (!correctObject.exists() || !predictedObject.exists())
      return;

    const TertiaryStructurePtr& predicted = predictedObject.getObjectAndCast<TertiaryStructure>(context);
    const TertiaryStructurePtr& correct = correctObject.getObjectAndCast<TertiaryStructure>(context);
    jassert(correct->getNumResidues() == predicted->getNumResidues());
    calphaRMSE->push(predicted->computeCAlphaAtomsRMSE(correct));
  }

protected:
  ScalarVariableMeanPtr calphaRMSE;
};

typedef ReferenceCountedObjectPtr<TertiaryStructureEvaluator> TertiaryStructureEvaluatorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_TERTIARY_STRUCTURE_H_
