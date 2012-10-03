/*-----------------------------------------.---------------------------------.
| Filename: VoteFunctions.h                | Voting Functions                |
| Author  : Francis Maes                   |                                 |
| Started : 22/01/2012 21:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_FUNCTION_VOTE_H_
# define LBCPP_ML_FUNCTION_VOTE_H_

# include <lbcpp/Luape/Function.h>
# include <lbcpp/Luape/Expression.h>
# include <lbcpp/Luape/LuapeCache.h> // for LuapeSampleVector

namespace lbcpp
{

class VoteFunction : public Function
{
public:
  VoteFunction(TypePtr outputType)
    : outputType(outputType) {}
  VoteFunction() {}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type == booleanType || type == probabilityType;}

  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return outputType;}

  virtual Variable computeVote(double input) const = 0;

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
    {return computeVote(inputs[0].toDouble());}

  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const std::vector<LuapeSampleVectorPtr>& inputs, TypePtr outputType) const
  {
    LuapeSampleVectorPtr weakPredictions = inputs[0];
    if (weakPredictions->getElementsType() == booleanType)
    {
      Variable negativeVote = computeVote(0.0);
      Variable positiveVote = computeVote(1.0);
      VectorPtr res = lbcpp::vector(outputType, weakPredictions->size());
      size_t index = 0;
      for (LuapeSampleVector::const_iterator it = weakPredictions->begin(); it != weakPredictions->end(); ++it)
      {
        unsigned char c = it.getRawBoolean();
        if (c < 2)
          res->setElement(index, c == 1 ? positiveVote : negativeVote);
        ++index;
      }
      return new LuapeSampleVector(weakPredictions->getIndices(), res);
    }
    else
    {
      jassert(weakPredictions->getElementsType() == probabilityType);
      VectorPtr res = lbcpp::vector(outputType, weakPredictions->size());
      size_t index = 0;
      for (LuapeSampleVector::const_iterator it = weakPredictions->begin(); it != weakPredictions->end(); ++it)
      {
        double d = it.getRawDouble();
        if (d != doubleMissingValue)
          res->setElement(index, computeVote(d));
        ++index;
      }
      return new LuapeSampleVector(weakPredictions->getIndices(), res);
    }
  }

  lbcpp_UseDebuggingNewOperator

protected:
  TypePtr outputType;
};

class ScalarVoteFunction : public VoteFunction
{
public:
  ScalarVoteFunction(double vote = 0.0)
    : VoteFunction(doubleType), vote(vote) {}

  virtual Variable computeVote(double input) const
    {return (input * 2 - 1) * vote;}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    if (inputs[0].exists())
      return computeVote(inputs[0].toDouble());
    else
      return 0.0;
  }

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "vote(" + inputs[0]->toShortString() + ", " + Variable(vote).toShortString() + ")";}

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

  virtual Variable computeVote(double input) const
  {
    DenseDoubleVectorPtr res = vote->cloneAndCast<DenseDoubleVector>();
    res->multiplyByScalar(input * 2 - 1);
    return res;
  }

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "vote(" + inputs[0]->toShortString() + ", " + vote->toShortString() + ")";}

protected:
  friend class VectorVoteFunctionClass;

  DenseDoubleVectorPtr vote;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_FUNCTION_VOTE_H_
