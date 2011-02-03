
#ifndef LBCPP_DECISION_TREE_R_TREE_INFERENCE_LEARNER_H_
# define LBCPP_DECISION_TREE_R_TREE_INFERENCE_LEARNER_H_

# include <lbcpp/Inference/InferenceBatchLearner.h>
# include <lbcpp/DecisionTree/RTreeInference.h>

namespace lbcpp
{

class RTreeInferenceLearner : public InferenceBatchLearner<RTreeInference>
{
public:  
  virtual ClassPtr getTargetInferenceClass() const
    {return rTreeInferenceClass;}
  
protected:
  friend class RTreeInferenceLearnerClass;

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const;
};

extern InferencePtr rTreeInferenceLearner();

}; /* namespace lbcpp */

#endif // !LBCPP_DECISION_TREE_R_TREE_INFERENCE_LEARNER_H_
