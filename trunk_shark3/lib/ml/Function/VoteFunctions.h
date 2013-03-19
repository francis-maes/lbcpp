/*-----------------------------------------.---------------------------------.
| Filename: VoteFunctions.h                | Voting Functions                |
| Author  : Francis Maes                   |                                 |
| Started : 22/01/2012 21:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_FUNCTION_VOTE_H_
# define ML_FUNCTION_VOTE_H_

# include <ml/Function.h>
# include <ml/Expression.h>

namespace lbcpp
{

class VoteFunction : public Function
{
public:
  VoteFunction(ClassPtr outputType)
    : outputType(outputType) {}
  VoteFunction() {}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const ClassPtr& type) const
    {return type == booleanClass || type == probabilityClass;}

  virtual ClassPtr initialize(const ClassPtr* inputTypes)
    {return outputType;}

  virtual ObjectPtr computeVote(double input) const = 0;

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (!inputs[0])
      return ObjectPtr();
    return computeVote(Double::get(inputs[0]));
  }

  virtual DataVectorPtr compute(ExecutionContext& context, const std::vector<DataVectorPtr>& inputs, ClassPtr outputType) const
  {
    DataVectorPtr weakPredictions = inputs[0];
    if (weakPredictions->getElementsType() == booleanClass)
    {
      ObjectPtr negativeVote = computeVote(0.0);
      ObjectPtr positiveVote = computeVote(1.0);
      VectorPtr res = lbcpp::vector(outputType, weakPredictions->size());
      size_t index = 0;
      for (DataVector::const_iterator it = weakPredictions->begin(); it != weakPredictions->end(); ++it)
      {
        unsigned char c = it.getRawBoolean();
        if (c < 2)
          res->setElement(index, c == 1 ? positiveVote : negativeVote);
        ++index;
      }
      return new DataVector(weakPredictions->getIndices(), res);
    }
    else
    {
      jassert(weakPredictions->getElementsType() == probabilityClass);
      VectorPtr res = lbcpp::vector(outputType, weakPredictions->size());
      size_t index = 0;
      for (DataVector::const_iterator it = weakPredictions->begin(); it != weakPredictions->end(); ++it)
      {
        double d = it.getRawDouble();
        if (d != DVector::missingValue)
          res->setElement(index, computeVote(d));
        ++index;
      }
      return new DataVector(weakPredictions->getIndices(), res);
    }
  }

  lbcpp_UseDebuggingNewOperator

protected:
  ClassPtr outputType;
};

class ScalarVoteFunction : public VoteFunction
{
public:
  ScalarVoteFunction(double vote = 0.0)
    : VoteFunction(doubleClass), vote(vote) {}

  virtual ObjectPtr computeVote(double input) const
    {return new Double((input * 2 - 1) * vote);}

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (inputs[0])
      return computeVote(Double::get(inputs[0]));
    else
      return new Double();
  }

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "vote(" + inputs[0]->toShortString() + ", " + ObjectPtr(new Double(vote))->toShortString() + ")";}

protected:
  friend class ScalarVoteFunctionClass;

  double vote;
};

class VectorVoteFunction : public VoteFunction
{
public:
  VectorVoteFunction(const DenseDoubleVectorPtr& vote)
    : VoteFunction(vote->getClass()), vote(vote) {}
  VectorVoteFunction() {}

  virtual ObjectPtr computeVote(double input) const
  {
    DenseDoubleVectorPtr res = vote->cloneAndCast<DenseDoubleVector>();
    res->multiplyByScalar(input * 2 - 1);
    return res;
  }

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "vote(" + inputs[0]->toShortString() + ", " + vote->toShortString() + ")";}

protected:
  friend class VectorVoteFunctionClass;

  DenseDoubleVectorPtr vote;
};

}; /* namespace lbcpp */

#endif // !ML_FUNCTION_VOTE_H_
