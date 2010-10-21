/*-----------------------------------------.---------------------------------.
| Filename: GraftingOnlineLearner.h        | Grafting Online Learner         |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2010 16:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_GRAFTING_H_
# define LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_GRAFTING_H_

# include <lbcpp/NumericalLearning/NumericalLearning.h>
# include "../../Perception/Modifier/SelectAndMakeProductsPerception.h"
# include "../Inference/LinearInference.h"
# include "../Inference/MultiLinearInference.h"

namespace lbcpp
{

class GraftingOnlineLearner : public InferenceOnlineLearner
{
public:
  GraftingOnlineLearner(PerceptionPtr perception, const std::vector<NumericalInferencePtr>& inferences);
  GraftingOnlineLearner() {}

  typedef std::vector<size_t> Conjunction;

  virtual void startLearningCallback();
  virtual void subStepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  virtual void stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  virtual void episodeFinishedCallback(const InferencePtr& inference) {}
  virtual void passFinishedCallback(const InferencePtr& inference);

  virtual bool isLearningStopped() const
    {return learningStopped;}

  virtual bool wantsMoreIterations() const
    {return !learningStopped;}

protected:
  void generateCandidates();
  void acceptCandidates();
  void pruneParameters();

  void resetCandidateScores();
  void updateCandidateScores(const NumericalInferencePtr& numericalInference, size_t firstScoreIndex, const Variable& input, const Variable& supervision, const Variable& prediction);

  void computeCandidateScores(std::vector<double>& res, Conjunction& bestCandidate, double& bestCandidateScore) const;
  double getCandidateScore(size_t candidateNumber, size_t scoreNumber) const;

  void computeActiveScores(std::vector<double>& res) const;

  String conjunctionToString(const Conjunction& conjunction) const;
  size_t getNumOutputs(const InferencePtr& inference) const;

protected:
  friend class GraftingOnlineLearnerClass;

  bool learningStopped;

  std::vector<NumericalInferencePtr> inferences;
  SelectAndMakeProductsPerceptionPtr perception;
  SelectAndMakeProductsPerceptionPtr candidatesPerception;

  // Candidate Score:
  //   max_{outputs} Score(Candidate, Output)
  std::vector< std::pair<ObjectPtr, size_t> > candidateScores;
  std::map<NumericalInferencePtr, size_t> scoresMapping; // inference -> Index of first score vector
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_GRAFTING_H_
