/*-----------------------------------------.---------------------------------.
| Filename: LocalitySensitiveHashingFu...h | Locality-Sensitive Hashing      |
| Author  : Julien Becker                  |                                 |
| Started : 13/09/2011 11:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LOCALITY_SENSITIVE_HASHING_FUNCTION_H_
# define LBCPP_LOCALITY_SENSITIVE_HASHING_FUNCTION_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Learning/NearestNeighbor.h>

namespace lbcpp
{

class LSHFunction : public Object
{
public:
  LSHFunction(const RandomGeneratorPtr& randomGenerator, const EnumerationPtr& enumeration, double segmentWidth)
    : randomVariable(new DenseDoubleVector(enumeration, doubleType)), bias(randomGenerator->sampleDouble(segmentWidth)), segmentWidth(segmentWidth)
  {
    // generate 2-Stable random variable
    const size_t n = enumeration->getNumElements();
    for (size_t j = 0; j < n; ++j)
      randomVariable->setValue(j, randomGenerator->sampleDoubleFromGaussian());
  }

  size_t computeHashValue(const DoubleVectorPtr& vector) const
    {return (size_t)floor((vector->dotProduct(randomVariable, 0) + bias) / segmentWidth);}

protected:
  friend class LSHFunctionClass;

  DenseDoubleVectorPtr randomVariable;
  double bias;
  double segmentWidth;

  LSHFunction() {}
};

typedef ReferenceCountedObjectPtr<LSHFunction> LSHFunctionPtr;

class LSHBucket : public Object
{
public:
  LSHBucket(const RandomGeneratorPtr& randomGenerator, const EnumerationPtr& enumeration, size_t numHashFunctions, size_t segmentWidth)
    : lshFunctions(numHashFunctions)
  {
    for (size_t i = 0; i < numHashFunctions; ++i)
      lshFunctions[i] = new LSHFunction(randomGenerator, enumeration, segmentWidth);
  }

  void addExample(size_t index, const DoubleVectorPtr& vector)
    {indicesMap.insert(std::make_pair(computeHashValue(vector), index));}

  void getIndicesSimilarTo(const DoubleVectorPtr& vector, std::vector<size_t>& results) const
  {
    typedef std::multimap<String, size_t>::const_iterator MapIterator;

    std::pair<MapIterator, MapIterator> elements = indicesMap.equal_range(computeHashValue(vector));
    for (MapIterator it = elements.first; it != elements.second; ++it)
      results.push_back(it->second);
  }

protected:
  friend class LSHBucketClass;

  std::vector<LSHFunctionPtr> lshFunctions;
  std::multimap<String, size_t> indicesMap;

  LSHBucket() {}

  String computeHashValue(const DoubleVectorPtr& vector) const
  {
    String res;
    for (size_t i = 0; i < lshFunctions.size(); ++i)
      res += String((int)lshFunctions[i]->computeHashValue(vector)) + T(",");
    return res;
  }
};

typedef ReferenceCountedObjectPtr<LSHBucket> LSHBucketPtr;

class LocalitySensitiveHashingBatchLearner;
extern BatchLearnerPtr localitySensitiveHashingBatchLearner();

class LocalitySensitiveHashingFunction : public Function
{
public:
  LocalitySensitiveHashingFunction()
    : maxNumNeighbors((size_t)-1), outerBallRadius(0.0)
    {setBatchLearner(filterUnsupervisedExamplesBatchLearner(localitySensitiveHashingBatchLearner()));}

  virtual TypePtr getSupervisionType() const = 0;

  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)doubleVectorClass() : getSupervisionType();}

  virtual String getOutputPostFix() const
    {return T("LSH");}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    DenseDoubleVectorPtr input = inputs[0].getObjectAndCast<DoubleVector>()->toDenseDoubleVector();
    std::vector<size_t> indices;
    for (size_t i = 0; i < buckets.size(); ++i)
      buckets[i]->getIndicesSimilarTo(input, indices);
    
    if (indices.size() == 0)
    {
      context.warningCallback(T("LocalitySensitiveHashingFunction::computeFunction"), T("No near neighbor found !"));
      return Variable::missingValue(getOutputType());
    }

    std::vector<size_t> randomIndices;
    if (indices.size() > maxNumNeighbors)
      context.getRandomGenerator()->sampleSubset(indices, maxNumNeighbors, randomIndices);
    else
      randomIndices = indices;
    

    std::vector<Variable> outputs;
    for (size_t i = 0; i < randomIndices.size(); ++i)
    {
      //std::cout << input->getDistanceTo(examples[randomIndices[i]], DenseDoubleVectorPtr()) << " <= " << outerBallRadius << std::endl;
      if (input->getDistanceTo(examples[randomIndices[i]], DenseDoubleVectorPtr()) <= outerBallRadius)
        outputs.push_back(supervisions[randomIndices[i]]);
    }

    return computeOutput(outputs);
  }

protected:
  friend class LocalitySensitiveHashingFunctionClass;
  friend class LocalitySensitiveHashingBatchLearner;

  std::vector<LSHBucketPtr> buckets;
  std::vector<DoubleVectorPtr> examples;
  std::vector<Variable> supervisions;

  size_t maxNumNeighbors;
  double outerBallRadius;

  virtual Variable computeOutput(const std::vector<Variable>& outputs) const = 0;
};

extern ClassPtr localitySensitiveHashingFunctionClass;
typedef ReferenceCountedObjectPtr<LocalitySensitiveHashingFunction> LocalitySensitiveHashingFunctionPtr;

class BinaryLocalitySensitiveHashing : public LocalitySensitiveHashingFunction
{
public:
  BinaryLocalitySensitiveHashing() {}

  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}

protected:
  friend class BinaryLocalitySensitiveHashingClass;

  virtual Variable computeOutput(const std::vector<Variable>& outputs) const
  {
    size_t numTrues = 0;
    for (size_t i = 0; i < outputs.size(); ++i)
      if (outputs[i].isBoolean() && outputs[i].getBoolean()
          || outputs[i].isDouble() && outputs[i].getDouble() > 0.5)
        ++numTrues;
    return probability(numTrues / (double)outputs.size());
  }
};

class LocalitySensitiveHashingBatchLearner : public BatchLearner
{
public:
  LocalitySensitiveHashingBatchLearner()
  {}

  virtual TypePtr getRequiredFunctionType() const
    {return localitySensitiveHashingFunctionClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const size_t numExamples = trainingData.size();
    double segmentWidth = 4.f;
    double epsilon = 0.9f;

    double approximationFactor = 1 + epsilon;

    double innerRadius = 1.f;
    double outerRadius = innerRadius * approximationFactor;

    double innerProb = ballProbability(segmentWidth);
    double outerProb = ballProbability(segmentWidth, approximationFactor);

    size_t numHashFunctions = (size_t)ceil(log((double)numExamples) / log(1.0 / outerProb));
    size_t numBuckets = (size_t)ceil(pow((double)numExamples, log(1.0 / innerProb) / log(1.0 / outerProb)));

    if (numExamples == 0)
    {
      context.errorCallback(T("No training examples !"));
      return false;
    }

    LocalitySensitiveHashingFunctionPtr lshFunction = function.staticCast<LocalitySensitiveHashingFunction>();
    EnumerationPtr elementsEnumeration = trainingData[0]->getVariable(0).getObjectAndCast<DoubleVector>()->getElementsEnumeration();
    // Initialize LSH
    lshFunction->maxNumNeighbors = 3 * numBuckets;
    lshFunction->outerBallRadius = outerRadius;
    // Create buckets
    lshFunction->buckets.resize(numBuckets);
    for (size_t i = 0; i < numBuckets; ++i)
      lshFunction->buckets[i] = new LSHBucket(context.getRandomGenerator(), elementsEnumeration, numHashFunctions, segmentWidth);

    // Initialize buckets
    lshFunction->examples.resize(numExamples);
    lshFunction->supervisions.resize(numExamples);
    for (size_t i = 0; i < numExamples; ++i)
    {
      const DoubleVectorPtr input = trainingData[i]->getVariable(0).getObjectAndCast<DoubleVector>();
      const Variable supervision = trainingData[i]->getVariable(1);
      lshFunction->examples[i] = input;
      lshFunction->supervisions[i] = supervision;
      for (size_t j = 0; j < numBuckets; ++j)
        lshFunction->buckets[j]->addExample(i, input);
    }
    return true;
  }

protected:
  friend class LocalitySensitiveHashingBatchLearnerClass;

  double ballProbability(const double segmentWidth, const double approximationFactor = 1.f) const
  {
    const double div = segmentWidth / approximationFactor;
    return 1 - 2 * (
                    standardNormalCumulativeDistributionFunction(-div)
                    + (1 - exp(-(segmentWidth * segmentWidth) / (2 * approximationFactor * approximationFactor))) / (sqrt(M_2_TIMES_PI) * div)
                    );
  }

  double standardNormalCumulativeDistributionFunction(double value) const
  {
    // http://en.wikipedia.org/wiki/Normal_distribution#Numerical_approximations_for_the_normal_CDF
    // Abramowitz & Stegun (1964)
    if (value < 0.f)
      return 1.f - standardNormalCumulativeDistributionFunction(-value);

    const double firstFactor = exp(-value * value / 2) / sqrt(M_2_TIMES_PI);
    const double t = 1 / (1 + 0.2316419f * value);
    double secondFactor = 0.31938153f * t;
    double nextT = t * t; // t^2
    secondFactor += -0.356563782f * nextT;
    nextT *= t; // t^3
    secondFactor += 1.781477937f * nextT;
    nextT *= t; // t^4
    secondFactor += -1.821255978 * nextT;
    nextT *= t; // t^5
    secondFactor += 1.33027442 * nextT;
    return 1.f - firstFactor * secondFactor;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LOCALITY_SENSITIVE_HASHING_FUNCTION_H_
