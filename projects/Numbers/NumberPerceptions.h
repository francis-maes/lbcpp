/*-----------------------------------------.---------------------------------.
| Filename: NumberPerceptions.h            | Number Related Perceptions      |
| Author  : Francis Maes                   |                                 |
| Started : 26/10/2010 18:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMBERS_PERCEPTION_H_
# define LBCPP_NUMBERS_PERCEPTION_H_

# include <lbcpp/Data/RandomGenerator.h>
# include <lbcpp/Data/Pair.h>
# include <lbcpp/Perception/Perception.h>
# include <lbcpp/NumericalLearning/NumericalLearning.h>

namespace lbcpp
{

inline ContainerPtr numberDifferencesSequence(const ContainerPtr& numberSequence)
{
  size_t n = numberSequence->getNumElements();
  if (!n)
    return vector(integerType, 0);
  ContainerPtr res = vector(integerType, n - 1);
  for (size_t i = 0; i < n - 1; ++i)
    res->setElement(i, numberSequence->getElement(i + 1).getInteger() - numberSequence->getElement(i).getInteger());
  return res;
}

inline ContainerPtr numberPairsSequence(const ContainerPtr& numberSequence)
{
  TypePtr elementsType = pairClass(integerType, integerType);
  size_t n = numberSequence->getNumElements();
  if (!n)
    return vector(elementsType, 0);
  ContainerPtr res = vector(elementsType, n - 1);
  for (size_t i = 0; i < n - 1; ++i)
    res->setElement(i, Variable::pair(numberSequence->getElement(i), numberSequence->getElement(i + 1)));
  return res;
}

inline ContainerPtr numberTripletsSequence(const ContainerPtr& numberSequence)
{
  TypePtr elementsType = pairClass(pairClass(integerType, integerType), integerType);
  size_t n = numberSequence->getNumElements();
  if (n < 2)
    return vector(elementsType, 0);
  ContainerPtr res = vector(elementsType, n - 2);
  for (size_t i = 0; i < n - 2; ++i)
    res->setElement(i, Variable::pair(
      Variable::pair(numberSequence->getElement(i), numberSequence->getElement(i + 1)),
      numberSequence->getElement(i + 2)));
  return res;
}

class EnrichedNumberSequence : public Object
{
public:
  EnrichedNumberSequence(ContainerPtr numbers)
    : numbers(numbers)
  {
    pairs = numberPairsSequence(numbers);
    triplets = numberTripletsSequence(numbers);
    differences = numberDifferencesSequence(numbers);
    differencesPairs = numberPairsSequence(differences);
  }
  EnrichedNumberSequence() {}

  const ContainerPtr& getNumbers() const
    {return numbers;}
  const ContainerPtr& getPairs() const
    {return pairs;}
  const ContainerPtr& getTriplets() const
    {return triplets;}
  const ContainerPtr& getDifferences() const
    {return differences;}
  const ContainerPtr& getDifferencesPairs() const
    {return differencesPairs;}

private:
  friend class EnrichedNumberSequenceClass;

  ContainerPtr numbers; // int
    ContainerPtr pairs;  // (int,int)
    ContainerPtr triplets; // ((int,int),int)
  ContainerPtr differences; // int
    ContainerPtr differencesPairs; // (int,int)
};

typedef ReferenceCountedObjectPtr<EnrichedNumberSequence> EnrichedNumberSequencePtr;
extern ClassPtr enrichedNumberSequenceClass;

extern PerceptionPtr containerSumFeatures(PerceptionPtr elementFeatures);
extern PerceptionPtr enrichedNumberSequencePerception(PerceptionPtr numbersPerception, PerceptionPtr pairsPerception, PerceptionPtr tripletsPerception);

class EnrichedNumberSequencePerception : public Perception
{
public:
  EnrichedNumberSequencePerception(PerceptionPtr numbersPerception, PerceptionPtr pairsPerception, PerceptionPtr tripletsPerception)
    : numbersPerception(numbersPerception), pairsPerception(pairsPerception), tripletsPerception(tripletsPerception) {}
  EnrichedNumberSequencePerception() {}

  virtual TypePtr getInputType() const
    {return enrichedNumberSequenceClass;}

  virtual void computeOutputType()
  {
    addOutputVariable(T("numbers"), numbersPerception);
    addOutputVariable(T("pairs"), pairsPerception);
    addOutputVariable(T("triplets"), tripletsPerception);
    addOutputVariable(T("differences"), numbersPerception);
    addOutputVariable(T("differencesPairs"), pairsPerception);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    const EnrichedNumberSequencePtr& sequence = input.getObjectAndCast<EnrichedNumberSequence>(context);
    callback->sense(0, numbersPerception, sequence->getNumbers());
    callback->sense(1, pairsPerception, sequence->getPairs());
    callback->sense(2, tripletsPerception, sequence->getTriplets());
    callback->sense(3, numbersPerception, sequence->getDifferences());
    callback->sense(4, pairsPerception, sequence->getDifferencesPairs());
  }

private:
  friend class EnrichedNumberSequencePerceptionClass;

  PerceptionPtr numbersPerception;
  PerceptionPtr pairsPerception;
  PerceptionPtr tripletsPerception;
};

class ContainerIsConstantFeature : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return containerClass(anyType);}

  virtual void computeOutputType()
  {
    addOutputVariable(T("isConstant"), doubleType);
    Perception::computeOutputType();
  }

 virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
 {
   const ContainerPtr& container = input.getObjectAndCast<Container>(context);
   if (container)
   {
     Variable refValue = container->getElement(0);
     for (size_t i = 1; i < container->getNumElements(); ++i)
       if (container->getElement(i) != refValue)
         return;
     callback->sense(0, 1.0);
   }
 }
};

class ContainerSumFeatures : public Perception
{
public:
  ContainerSumFeatures(PerceptionPtr elementFeatures)
    : elementFeatures(elementFeatures) {}
  ContainerSumFeatures() {}

  virtual TypePtr getInputType() const
    {return containerClass(elementFeatures->getInputType());}

  virtual void computeOutputType()
  {
    addOutputVariable(T("sum"), elementFeatures);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    const ContainerPtr& container = input.getObjectAndCast<Container>(context);
    size_t n = container->getNumElements();
    
    //ObjectPtr sum;
    for (size_t i = 0; i < n; ++i)
      callback->sense(0, elementFeatures, container->getElement(i));
      //lbcpp::addWeighted(sum, elementFeatures, container->getElement(i), 1.0);
    //if (sum)
     // callback->sense(0, sum);
  }

protected:
  friend class ContainerSumFeaturesClass;
  PerceptionPtr elementFeatures;
};

//////////////////////////////////////////////////////////////////
class NumbersSerieProblem;
typedef ReferenceCountedObjectPtr<NumbersSerieProblem> NumbersSerieProblemPtr;
extern ClassPtr numbersSerieProblemClass;

class NumbersSerieProblem : public Object
{
public:
  NumbersSerieProblem(const std::vector<int>& previousNumbers, int target)
    : previousNumbers(previousNumbers), target(target) {}
  NumbersSerieProblem(const String& description)
  {
    StringArray tokens;
    tokens.addTokens(description, true);
    previousNumbers.resize(tokens.size() - 1);
    for (size_t i = 0; i < previousNumbers.size(); ++i)
      previousNumbers[i] = tokens[i].getIntValue();
    target = tokens.size() ? tokens[tokens.size() - 1].getIntValue() : 0;
  }

  NumbersSerieProblem() {}

  static NumbersSerieProblemPtr randomSample(RandomGeneratorPtr random, int minValue = 0, int maxValue = 100, size_t numPreviousNumbers = 5)
  {
    std::vector<int> previousNumbers(numPreviousNumbers);
    for (size_t i = 0; i < previousNumbers.size(); ++i)
      previousNumbers[i] = random->sampleInt(minValue, maxValue);
    int target = random->sampleInt(minValue, maxValue);
    return new NumbersSerieProblem(previousNumbers, target);
  }
  
  Variable createRankingExample(int minValue = 0, int maxValue = 100) const
  {
    jassert(maxValue > minValue);
    size_t n = (size_t)(maxValue - minValue);

    ContainerPtr alternatives = vector(numbersSerieProblemClass, n);
    ContainerPtr costs = vector(doubleType, n);

    for (size_t i = 0; i < n; ++i)
    {
      int targetAlternative = minValue + (int)i;
      alternatives->setElement(i, new NumbersSerieProblem(previousNumbers, targetAlternative));
      costs->setElement(i, targetAlternative == target ? 0.0 : 1.0);
    }
    return Variable::pair(alternatives, costs);
  }

  ContainerPtr createRankingDataForAllSubProblems(int minValue = 0, int maxValue = 100) const
  {
    if (!previousNumbers.size())
      return ContainerPtr();

    TypePtr rankingExampleType = pairClass(containerClass(numbersSerieProblemClass), containerClass(doubleType));
    ContainerPtr res = vector(rankingExampleType, previousNumbers.size() - 1);
    
    for (size_t i = 0; i < previousNumbers.size() - 1; ++i)
    {
      std::vector<int> subNumbers = previousNumbers;
      subNumbers.resize(i + 1);
      NumbersSerieProblemPtr subProblem = new NumbersSerieProblem(subNumbers, previousNumbers[i + 1]);
      res->setElement(i, subProblem->createRankingExample(minValue, maxValue));
    }
    return res;    
  }

  int getTargetNumber() const
    {return target;}

  size_t getNumPreviousNumbers() const
    {return previousNumbers.size();}

  size_t getLastPreviousNumber() const
    {jassert(previousNumbers.size()); return previousNumbers.back();}

private:
  friend class NumbersSerieProblemClass;

  std::vector<int> previousNumbers;
  int target;
};

class NumbersSerieProblemGetNumbersPairFunction : public Function
{
public:
  virtual TypePtr getInputType() const
    {return numbersSerieProblemClass;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return pairClass(integerType, integerType);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const NumbersSerieProblemPtr& problem = input.getObjectAndCast<NumbersSerieProblem>(context);
    return Variable::pair(problem->getTargetNumber(), problem->getLastPreviousNumber());
  }
};

extern PerceptionPtr numberPairDifferencePerception(PerceptionPtr differencePerception);

extern PerceptionPtr singleDigitNumberFeatures();
extern PerceptionPtr twoDigitNumberFeatures();

extern PerceptionPtr numberPairIsMultipleFeatures(int firstMultiple, int lastMultiple);
extern PerceptionPtr pairBidirectionalPerception(PerceptionPtr directionalPerception);

class SingleDigitNumberFeatures : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return integerType;}
  
  virtual void computeOutputType()
  {
    reserveOutputVariables(19);
    for (int i = -9; i <= 9; ++i)
      addOutputVariable(T("equalsTo") + String(i), doubleType);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    int i = input.getInteger();
    if (i > -10 && i < 10)
      callback->sense(i + 9, 1.0);
  }
};

class TwoDigitNumberFeatures : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return integerType;}
  
  virtual void computeOutputType()
  {
    reserveOutputVariables(199);
    for (int i = -99; i <= 99; ++i)
      addOutputVariable(T("equalsTo") + String(i), doubleType);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    int i = input.getInteger();
    if (i > -100 && i < 100)
      callback->sense(i + 99, 1.0);
  }
};

class NumberPairDifferencePerception : public Perception
{
public:
  NumberPairDifferencePerception(PerceptionPtr differencePerception = PerceptionPtr())
    : differencePerception(differencePerception) {}

  virtual TypePtr getInputType() const
    {return pairClass(integerType, integerType);}

  virtual void computeOutputType()
  {
    addOutputVariable(T("difference"), differencePerception);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>(context);    
    int a = pair->getFirst().getInteger();
    int b = pair->getSecond().getInteger();
    callback->sense(0, differencePerception, b - a);
  }

protected:
  friend class NumberPairDifferencePerceptionClass;

  PerceptionPtr differencePerception;
};

class NumberPairIsMultipleFeatures : public Perception
{
public:
  NumberPairIsMultipleFeatures(int firstMultiple = 2, int lastMultiple = 20)
    : firstMultiple(firstMultiple), lastMultiple(lastMultiple)
  {
    jassert(firstMultiple <= lastMultiple);
  }

  virtual TypePtr getInputType() const
    {return pairClass(integerType, integerType);}

  virtual void computeOutputType()
  {
    reserveOutputVariables(lastMultiple - firstMultiple + 2);
    for (int i = firstMultiple; i <= lastMultiple; ++i)
      addOutputVariable(T("multiple") + String((int)i), doubleType);
    addOutputVariable(T("otherMultiple"), doubleType);
    addOutputVariable(T("notMultiple"), doubleType);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>(context);    
    int a = pair->getFirst().getInteger();
    int b = pair->getSecond().getInteger();
    int m;
    
    size_t idx = getNumOutputVariables() - 2;
    if (isMultiple(a, b, m))
    {
      if (m >= firstMultiple && m <= lastMultiple)
        callback->sense(m - firstMultiple, 1.0);
      else if (m != 1)
        callback->sense(idx, 1.0);
    }
    else
      callback->sense(idx + 1, 1.0);
  }

  bool isMultiple(int a, int b, int& m) const
  {
    if (a == 0 || b == 0)
      return false;
    bool sameSign = (a < 0) == (b < 0);
    a = abs(a);
    b = abs(b);
    if (a % b != 0)
      return false;
    m = a / b;
    if (!sameSign)
      m = -m;
    return true;
  }

private:
  friend class NumberPairIsMultipleFeaturesClass;

  int firstMultiple;
  int lastMultiple;
};

class PairBidirectionalPerception : public Perception
{
public:
  PairBidirectionalPerception(PerceptionPtr directionalPerception)
    : directionalPerception(directionalPerception) {}
  PairBidirectionalPerception() {}

  virtual TypePtr getInputType() const
    {return pairClass(anyType, anyType);}

  virtual void computeOutputType()
  {
    addOutputVariable(T("forward"), directionalPerception);
    addOutputVariable(T("backward"), directionalPerception);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>(context);    
    callback->sense(0, directionalPerception, pair);
    callback->sense(1, directionalPerception, new Pair(pair->getSecond(), pair->getFirst()));
  }

protected:
  friend class PairBidirectionalPerceptionClass;

  PerceptionPtr directionalPerception;
};

};

#endif // !LBCPP_NUMBERS_PERCEPTION_H_