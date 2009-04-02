/*-----------------------------------------.---------------------------------.
| Filename: LearningExample.h              | Learning examples               |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 16:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_LEARNING_EXAMPLE_H_
# define LCPP_LEARNING_EXAMPLE_H_

# include "FeatureGenerator.h"
# include "EditableFeatureGenerator.h"
# include "Utilities.h"

namespace lcpp
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

class LearningExamplesParser : public TextFileParser
{
public:
  LearningExamplesParser() : dictionary(NULL) {}

  bool parse(std::istream& istr, FeatureDictionaryPtr dictionary);
  
protected:
  FeatureDictionaryPtr dictionary;

  // featureList ::= feature featureList | feature
  bool parseFeatureList(const std::vector<std::string>& columns, size_t firstColumn, SparseVectorPtr& res);
  // feature ::= featureId . featureValue
  static bool parseFeature(const std::string& str, std::string& featureId, double& featureValue);
    // featureId ::= name . featureId  | name
  static bool parseFeatureIdentifier(const std::string& identifier, std::vector<std::string>& path);
};

extern bool parseClassificationExamples(std::istream& istr, FeatureDictionaryPtr dictionary, StringDictionaryPtr labels, std::vector<ClassificationExample>& res);
extern bool parseRegressionExamples(std::istream& istr, FeatureDictionaryPtr dictionary, std::vector<RegressionExample>& res);


}; /* namespace lcpp */

#endif // !LCPP_LEARNING_EXAMPLE_H_
