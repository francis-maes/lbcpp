/*-----------------------------------------.---------------------------------.
| Filename: VoteLuapeFunctions.h           | Voting Luape Functions          |
| Author  : Francis Maes                   |                                 |
| Started : 22/01/2012 21:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FUNCTION_VOTE_H_
# define LBCPP_LUAPE_FUNCTION_VOTE_H_

# include <lbcpp/Luape/LuapeFunction.h>
# include <lbcpp/Luape/LuapeNode.h>
# include <lbcpp/Luape/LuapeCache.h> // for LuapeSampleVector

namespace lbcpp
{

class VoteLuapeFunction : public LuapeFunction
{
public:
  VoteLuapeFunction(TypePtr outputType)
    : outputType(outputType) {}
  VoteLuapeFunction() {}

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

class ScalarVoteLuapeFunction : public VoteLuapeFunction
{
public:
  ScalarVoteLuapeFunction(double vote = 0.0)
    : VoteLuapeFunction(doubleType), vote(vote) {}

  virtual Variable computeVote(double input) const
    {return (input * 2 - 1) * vote;}

  virtual String makeNodeName(const std::vector<LuapeNodePtr>& inputs) const
    {return "vote(" + inputs[0]->toShortString() + ", " + Variable(vote).toShortString() + ")";}

protected:
  friend class ScalarVoteLuapeFunctionClass;

  double vote;
};

class VectorVoteLuapeFunction : public VoteLuapeFunction
{
public:
  VectorVoteLuapeFunction(const DenseDoubleVectorPtr& vote)
    : VoteLuapeFunction(vote->getClass()), vote(vote) {}
  VectorVoteLuapeFunction() {}

  virtual Variable computeVote(double input) const
  {
    DenseDoubleVectorPtr res = vote->cloneAndCast<DenseDoubleVector>();
    res->multiplyByScalar(input * 2 - 1);
    return res;
  }

  virtual String makeNodeName(const std::vector<LuapeNodePtr>& inputs) const
    {return "vote(" + inputs[0]->toShortString() + ", " + vote->toShortString() + ")";}

protected:
  friend class VectorVoteLuapeFunctionClass;

  DenseDoubleVectorPtr vote;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FUNCTION_VOTE_H_
