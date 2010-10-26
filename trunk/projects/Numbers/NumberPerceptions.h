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

namespace lbcpp
{

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

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    const NumbersSerieProblemPtr& problem = input.getObjectAndCast<NumbersSerieProblem>();
    return Variable::pair(problem->getTargetNumber(), problem->getLastPreviousNumber());
  }
};

extern PerceptionPtr numberPairIsMultipleFeatures(int firstMultiple, int lastMultiple);
extern PerceptionPtr pairBidirectionalPerception(PerceptionPtr directionalPerception);
  

  
class NumberPairIsMultipleFeatures : public Perception
{
public:
  NumberPairIsMultipleFeatures(int firstMultiple = 1, int lastMultiple = 20)
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
      addOutputVariable(T("a=") + String((int)i) + T("b"), doubleType);
    addOutputVariable(T("OtherMultiple"), doubleType);
    addOutputVariable(T("NotMultiple"), doubleType);
    Perception::computeOutputType();
  }

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>();    
    int a = pair->getFirst().getInteger();
    int b = pair->getSecond().getInteger();
    int m;
    
    size_t idx = getNumOutputVariables() - 2;
    if (isMultiple(a, b, m))
    {
      if (m >= firstMultiple && m <= lastMultiple)
        callback->sense(m - firstMultiple, 1.0);
      else
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

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>();    
    callback->sense(0, directionalPerception, pair);
    callback->sense(1, directionalPerception, new Pair(pair->getSecond(), pair->getFirst()));
  }

protected:
  friend class PairBidirectionalPerceptionClass;

  PerceptionPtr directionalPerception;
};

};

#endif // !LBCPP_NUMBERS_PERCEPTION_H_