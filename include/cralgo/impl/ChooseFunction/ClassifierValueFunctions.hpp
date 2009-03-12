/*-----------------------------------------.---------------------------------.
| Filename: ClassifierValueFunction.hpp    | Classifier-based value functions|
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 16:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_VALUE_FUNCTION_CLASSIFIER_H_
# define CRALGO_IMPL_VALUE_FUNCTION_CLASSIFIER_H_

# include "ChooseFunctionStatic.hpp"
# include "../../LearningMachine.h"

namespace cralgo {
namespace impl {

template<class ExactType>
struct ClassifierBasedActionValueFunction : public ActionValueFunction<ExactType, size_t>
{
  ClassifierBasedActionValueFunction(ClassifierPtr classifier)
    : classifier(classifier) {}
    
  ClassifierPtr classifier;
  DenseVectorPtr scores;
    
  double compute(size_t choice) const
  {
    assert(scores && choice < scores->getNumValues());
    return scores->get(choice);
  }
};

struct ClassifierScoresActionValue : public ClassifierBasedActionValueFunction<ClassifierScoresActionValue>
{
  typedef ClassifierBasedActionValueFunction<ClassifierScoresActionValue> BaseClass;
  
  ClassifierScoresActionValue(ClassifierPtr classifier)
    : BaseClass(classifier) {}
    
  void setChoose(ChoosePtr choose)
    {scores = classifier->predictScores(choose->stateFeatures());}
};

struct ClassifierProbabilitiesActionValue  : public ClassifierBasedActionValueFunction<ClassifierProbabilitiesActionValue>
{
  typedef ClassifierBasedActionValueFunction<ClassifierProbabilitiesActionValue> BaseClass;
  
  ClassifierProbabilitiesActionValue(ClassifierPtr classifier)
    : BaseClass(classifier) {}
    
  void setChoose(ChoosePtr choose)
    {scores = classifier->predictProbabilities(choose->stateFeatures());}
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_VALUE_FUNCTION_CLASSIFIER_H_