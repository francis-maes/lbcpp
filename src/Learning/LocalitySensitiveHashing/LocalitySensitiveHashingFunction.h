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
  LSHBucket(const RandomGeneratorPtr& randomGenerator, const EnumerationPtr& enumeration, size_t numHashFunctions, double segmentWidth)
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
  LocalitySensitiveHashingFunction(size_t numNeighbors = 1)
    : numNeighbors(numNeighbors)
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

    // LSH Ball
    std::vector<bool> isScoreAlreaydComputed(examples.size(), false);
    ScoresMap scores;
    for (size_t i = 0; i < indices.size(); ++i)
    {
      if (isScoreAlreaydComputed[indices[i]])
        continue;
      scores.insert(std::pair<double, size_t>((size_t)input->getDistanceTo(examples[indices[i]]->toSparseVector()), indices[i]));
    }
    
#if 0
    std::cout << "Found indices: " << indices.size() << " - In the ball: " << outputs.size();
    // Exact Ball
    const double ballRadius = 1.f;
    size_t numElementsInExactBall = 0;
    for (size_t i = 0; i < computedScores.size(); ++i)
    {
      if (computedScores[i] == DBL_MAX)
        computedScores[i] = input->getDistanceTo(examples[i]->toSparseVector(), DenseDoubleVectorPtr());
      if (computedScores[i] <= ballRadius)
        ++numElementsInExactBall;
    }
    std::cout << " - Exact ball: " << numElementsInExactBall << std::endl;
    return probability(numElementsInExactBall ? outputs.size() / (double)numElementsInExactBall : 1.f);
#endif // !0
    return computeOutput(scores);
  }

protected:
  friend class LocalitySensitiveHashingFunctionClass;
  friend class LocalitySensitiveHashingBatchLearner;

  typedef std::multimap<double, size_t> ScoresMap;

  size_t numNeighbors;

  std::vector<LSHBucketPtr> buckets;
  std::vector<DoubleVectorPtr> examples;
  std::vector<Variable> supervisions;

  virtual Variable computeOutput(const ScoresMap& outputs) const = 0;
};

extern ClassPtr localitySensitiveHashingFunctionClass;
typedef ReferenceCountedObjectPtr<LocalitySensitiveHashingFunction> LocalitySensitiveHashingFunctionPtr;

class BinaryLocalitySensitiveHashing : public LocalitySensitiveHashingFunction
{
public:
  BinaryLocalitySensitiveHashing(size_t numNeighbors = 1)
    : LocalitySensitiveHashingFunction(numNeighbors) {}

  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}

protected:
  friend class BinaryLocalitySensitiveHashingClass;

  virtual Variable computeOutput(const ScoresMap& outputs) const
  {
    const size_t maxNumNeighbors = outputs.size() < numNeighbors ? outputs.size() : numNeighbors;
    ScoresMap::const_iterator it = outputs.begin();
    size_t numTrues = 0;
    for (size_t i = 0; i < maxNumNeighbors; ++i, it++)
    {
      const Variable v = supervisions[it->second];
      if (v.isBoolean() && v.getBoolean() || v.isDouble() && v.getDouble() > 0.5)
        ++numTrues;
    }
    return probability(numTrues / (double)maxNumNeighbors);
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

    if (numExamples == 0)
    {
      context.errorCallback(T("No training examples !"));
      return false;
    }

    double segmentWidth = 4.f;
    double successProbability = 0.999f;
    double ballRadius = 1.f;

    LocalitySensitiveHashingFunctionPtr lshFunction = function.staticCast<LocalitySensitiveHashingFunction>();
    // Load examples
    lshFunction->examples.resize(numExamples);
    lshFunction->supervisions.resize(numExamples);
    for (size_t i = 0; i < numExamples; ++i)
    {
      const DoubleVectorPtr input = trainingData[i]->getVariable(0).getObjectAndCast<DoubleVector>();
      const Variable supervision = trainingData[i]->getVariable(1);
      lshFunction->examples[i] = input;
      lshFunction->supervisions[i] = supervision;
    }

    const double collisionDensity = p2StableCollisionDensity(segmentWidth);
    const size_t numHashFunctions = computeNumHashFunctions(context, segmentWidth, successProbability, ballRadius, lshFunction->examples, context.getRandomGenerator(), 100);
    const size_t numBuckets = computeNumBuckets(collisionDensity, numHashFunctions, successProbability);

    EnumerationPtr elementsEnumeration = trainingData[0]->getVariable(0).getObjectAndCast<DoubleVector>()->getElementsEnumeration();
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

  double p2StableCollisionDensity(const double segmentWidth, const double l2Norm = 1.f) const
  {
    const double div = segmentWidth / l2Norm;
    return 1 - 2 * (
                    standardNormalCumulativeDistribution(-div)
                    + (1 - exp(-(segmentWidth * segmentWidth) / (2 * l2Norm * l2Norm))) / (sqrt(M_2_TIMES_PI) * div)
                    );
  }

  double standardNormalCumulativeDistribution(double value) const
  {
    // http://en.wikipedia.org/wiki/Normal_distribution#Numerical_approximations_for_the_normal_CDF
    // Abramowitz & Stegun (1964)
    if (value < 0.f)
      return 1.f - standardNormalCumulativeDistribution(-value);

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

  size_t reducedNumHashFunctions(const double collisionDensity, const size_t numHashFunctions, const double successProbability) const
  {
    const double prob = pow(collisionDensity, numHashFunctions / 2.0);
    size_t res = 1;
    while (pow(1-prob, (double)res) + res * prob * pow(1-prob, res-1.0) > 1 - successProbability)
      ++res;
    return res;
  }

  size_t computeNumBuckets(const double collisionDensity, const size_t numHashFunctions, const double successProbability) const
  {
    return (size_t)ceil(log(1 - successProbability) / log(1 - pow(collisionDensity, (double)numHashFunctions)));
  }

  double expectedNumCollisionsPerBucket(const double segmentWidth, const size_t numHashFunctions, const double ballRadius,
                                        const std::vector<DoubleVectorPtr>& data, const RandomGeneratorPtr& rand, const size_t numSamples) const
  {
    double res = 0.f;
    const DenseDoubleVectorPtr ddv = data[rand->sampleSize(data.size())]->toDenseDoubleVector();
    for (size_t i = 0; i < numSamples; ++i)
    {
      const SparseDoubleVectorPtr sdv = data[rand->sampleSize(data.size())]->toSparseVector();
      const double prob = p2StableCollisionDensity(segmentWidth, ddv->getDistanceTo(sdv) / ballRadius);
      res += pow(prob, (double)numHashFunctions);
    }
    return res;
  }

  size_t computeNumHashFunctions(ExecutionContext& context, const double segmentWidth, const double successProbability, const double ballRadius,
                                 const std::vector<DoubleVectorPtr>& data, const RandomGeneratorPtr& rand, const size_t numSamples) const
  {
    const double collisionDensity = p2StableCollisionDensity(segmentWidth);
    context.enterScope(T("Compute Number of Hash Functions"));

    size_t numHashFunctions = 1;
    double previousScore = DBL_MAX;
    double score = DBL_MAX;
    do
    {
      const size_t numBuckets = computeNumBuckets(collisionDensity, numHashFunctions, successProbability);
      const double numCollisions = expectedNumCollisionsPerBucket(segmentWidth, numHashFunctions, ballRadius, data, rand, numSamples);
      previousScore = score;
      score = numBuckets * (numHashFunctions + numCollisions);
  
      context.enterScope(T("Iteration") + String((int)numHashFunctions));
      context.resultCallback(T("numHashFunctions"), numHashFunctions);
      context.resultCallback(T("numBuckets"), numBuckets);
      context.resultCallback(T("numBuckets * numHashFunctions"), numHashFunctions * numBuckets);
      context.resultCallback(T("numCollisions/Buckets"), numCollisions);
      context.resultCallback(T("totalNumCollisions"), numBuckets * numCollisions);
      context.resultCallback(T("numHashFunctions + numCollisions"), numCollisions + numHashFunctions);
      context.resultCallback(T("totalHashFunctions + totalCollisions"), score);
      context.leaveScope();
      
      ++numHashFunctions;
    } while (score < previousScore);

    context.leaveScope(numHashFunctions - 2);

    return numHashFunctions - 2;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LOCALITY_SENSITIVE_HASHING_FUNCTION_H_
