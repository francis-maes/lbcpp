/*-----------------------------------------.---------------------------------.
| Filename: LearningExample.h              | Learning examples               |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 16:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_EXAMPLE_H_
# define LBCPP_LEARNING_EXAMPLE_H_

# include "FeatureGenerator.h"
# include "EditableFeatureGenerator.h"
# include "Utilities.h"

namespace lbcpp
{

class LearningExample : public Object
{
public:
  LearningExample(const LearningExample& other) : weight(other.weight) {}
  LearningExample() : weight(1.0) {}
  
  double getWeight() const
    {return weight;}
    
private:
  double weight;
};

typedef ReferenceCountedObjectPtr<LearningExample> LearningExamplePtr;

class ClassificationExample : public LearningExample
{
public:
  ClassificationExample(const ClassificationExample& other)
    : LearningExample(other), x(other.x), y(other.y) {}
  ClassificationExample(FeatureGeneratorPtr x, size_t y)
    : x(x), y(y) {}
  ClassificationExample()
    : y(0) {}
    
  double getMarginMultiplier() const // binary classification
    {return y == 0 ? -1.0 : 1.0;}
  
  FeatureGeneratorPtr getInput() const
    {return x;}
    
  size_t getOutput() const
    {return y;}
  
  friend std::ostream& operator << (std::ostream& ostr, const ClassificationExample& example)
    {return ostr << "Y = " << example.y << " X = " << example.x->toString();}
  
private:
  FeatureGeneratorPtr x;
  size_t y;
};

typedef ReferenceCountedObjectPtr<ClassificationExample> ClassificationExamplePtr;

class GeneralizedClassificationExample : public LearningExample
{
public:
  GeneralizedClassificationExample(const FeatureGeneratorPtr alternatives, size_t output)
    : alternatives(alternatives), output(output) {}

  FeatureGeneratorPtr getInput() const
    {return alternatives;}
    
  size_t getOutput() const
    {return output;}

  size_t getNumAlternatives() const
    {return alternatives->getNumSubGenerators();}
    
  FeatureGeneratorPtr getAlternative(size_t index) const
    {return alternatives->getSubGeneratorWithIndex(index);}
  
private:
  FeatureGeneratorPtr alternatives;
  size_t output;
};

typedef ReferenceCountedObjectPtr<GeneralizedClassificationExample> GeneralizedClassificationExamplePtr;

class RegressionExample : public LearningExample
{
public:
  RegressionExample(FeatureGeneratorPtr x, double y)
    : x(x), y(y) {}
    
  FeatureGeneratorPtr getInput() const
    {return x;}
    
  double getOutput() const
    {return y;}

  friend std::ostream& operator << (std::ostream& ostr, const RegressionExample& example)
    {return ostr << "Y = " << example.y << " X = " << example.x->toString();}

private:
  FeatureGeneratorPtr x;
  double y;
};

typedef ReferenceCountedObjectPtr<RegressionExample> RegressionExamplePtr;

class RankingExample : public LearningExample
{
public:
  RankingExample(const FeatureGeneratorPtr alternatives, const std::vector<double>& costs)
    : alternatives(alternatives), costs(costs) {}

  FeatureGeneratorPtr getInput() const
    {return alternatives;}
    
  const std::vector<double>& getCosts() const
    {return costs;}
    
private:
  FeatureGeneratorPtr alternatives;
  std::vector<double> costs;
};

typedef ReferenceCountedObjectPtr<RankingExample> RankingExamplePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_EXAMPLE_H_
