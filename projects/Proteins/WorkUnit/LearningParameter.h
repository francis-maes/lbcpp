
#ifndef LBCPP_PROTEIN_LEARNING_PARAMETER_H_
# define LBCPP_PROTEIN_LEARNING_PARAMETER_H_

# include <lbcpp/Core/Object.h>
# include <lbcpp/Core/Variable.h>
# include <lbcpp/Distribution/ContinuousDistribution.h>
# include <lbcpp/Distribution/MultiVariateDistribution.h>
# include <lbcpp/Inference/InferenceOnlineLearner.h>

namespace lbcpp
{

class LearningParameter : public InferenceOnlineLearnerParameters
{
public:
  virtual IndependentMultiVariateDistributionPtr getAprioriDistribution() const = 0;
  virtual InferenceOnlineLearnerPtr createLearner() const {return InferenceOnlineLearnerPtr();} // FIXME: ExtraTree
};

typedef ReferenceCountedObjectPtr<LearningParameter> LearningParameterPtr;

class NumericalLearningParameter : public LearningParameter
{
public:
  NumericalLearningParameter(double learningRate, double learningRateDecrease, double regularizer)
    : learningRate(learningRate), learningRateDecrease(learningRateDecrease), regularizer(regularizer) {}
  NumericalLearningParameter()
    : learningRate(0.0), learningRateDecrease(0.0), regularizer(0.0) {}

  double getLearningRate() const
    {return pow(10.0, learningRate);}
  
  size_t getLearningRateDecrease() const
    {return (size_t)pow(10.0, learningRateDecrease);}
  
  double getRegularizer() const
    {return (regularizer <= -10.0) ? 0 : pow(10.0, regularizer);}
  
  virtual IndependentMultiVariateDistributionPtr getAprioriDistribution() const
  {
    IndependentMultiVariateDistributionPtr apriori = new IndependentMultiVariateDistribution(getClass());
    apriori->setSubDistribution(0, new UniformDistribution(-5, 5));  // TODO: Logarithmic distribution
    apriori->setSubDistribution(1, new UniformDistribution(0, 7));   // TODO: Logarithmic distribution
    apriori->setSubDistribution(2, new UniformDistribution(-10, 0)); // TODO: Logarithmic distribution
    return apriori;
  }
  
  virtual InferenceOnlineLearnerPtr createLearner() const
  {
    return learner->cloneAndCast<InferenceOnlineLearner>();
  }
  
  virtual void setLearner(InferenceOnlineLearnerPtr learner)
    {this->learner = learner;}

  virtual bool loadFromString(ExecutionContext& context, const String& value)
  {
    StringArray values;
    values.addTokens(value, T(";"), T(""));
    if (values.size() != 3)
    {
      context.errorCallback(T("NumericalLearningParameter::loadFromString"), value.quoted() + T(" is not valid"));
      return false;
    }
    
    Variable vLR  = Variable::createFromString(context, doubleType, values[0]);
    Variable vLRD = Variable::createFromString(context, doubleType, values[1]);
    Variable vR   = Variable::createFromString(context, doubleType, values[2]);
    
    if (vLR.isMissingValue() || vLRD.isMissingValue() || vR.isMissingValue())
    {
      context.errorCallback(T("NumericalLearningParameter::loadFromString"), value.quoted() + T(" is not valid"));
      return false;
    }
    
    learningRate = vLR.getDouble();
    learningRateDecrease = vLRD.getDouble();
    regularizer = vR.getDouble();
    return true;
  }

protected:
  friend class NumericalLearningParameterClass;
  
  double learningRate;
  double learningRateDecrease;
  double regularizer;
  
  InferenceOnlineLearnerPtr learner;
};

typedef ReferenceCountedObjectPtr<NumericalLearningParameter> NumericalLearningParameterPtr;

class ExtraTreeLearningParameter : public LearningParameter
{
public:
  ExtraTreeLearningParameter(size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
    : numTrees(numTrees), numAttributeSamplesPerSplit(numAttributeSamplesPerSplit), minimumSizeForSplitting(minimumSizeForSplitting) {}
  ExtraTreeLearningParameter()
    : numTrees(0), numAttributeSamplesPerSplit(0), minimumSizeForSplitting(0) {}

  size_t getNumTrees() const
    {return numTrees;}
  
  size_t getNumAttributeSamplesPerSplit() const
    {return numAttributeSamplesPerSplit;}
  
  size_t getMinimumSizeForSplitting() const
    {return minimumSizeForSplitting;}
  
  virtual IndependentMultiVariateDistributionPtr getAprioriDistribution() const
  {
    IndependentMultiVariateDistributionPtr apriori = new IndependentMultiVariateDistribution(getClass());
    apriori->setSubDistribution(0, new UniformDistribution(1, 1000)); // TODO: DiscreteUniformDistribution
    apriori->setSubDistribution(1, new UniformDistribution(1, 400));  // TODO: DiscreteUniformDistribution
    apriori->setSubDistribution(2, new UniformDistribution(2, 500));  // TODO: DiscreteUniformDistribution
    return apriori;
  }
  
protected:
  friend class ExtraTreeLearningParameterClass;
  
  size_t numTrees;
  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;
};

typedef ReferenceCountedObjectPtr<ExtraTreeLearningParameter> ExtraTreeLearningParameterPtr;

}; /* namspace */

#endif // !LBCPP_PROTEIN_LEARNING_PARAMETER_H_
