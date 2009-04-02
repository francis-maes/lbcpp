/*-----------------------------------------.---------------------------------.
| Filename: SyntheticDataGenerator.h       | Synthetic Data Generator        |
| Author  : Francis Maes                   |                                 |
| Started : 29/03/2009 22:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef SYNTHETIC_DATA_GENERATOR_H_
# define SYNTHETIC_DATA_GENERATOR_H_

#include <lcpp/lcpp.h>

namespace lcpp
{

class SyntheticLinearMultiClassGenerator
{
public:
  SyntheticLinearMultiClassGenerator(size_t numFeatures, size_t numClasses)
    : parameters(numClasses), numFeatures(numFeatures)
  {
    for (size_t i = 0; i < parameters.size(); ++i)
      parameters[i] = sampleVectorGaussian(numFeatures);
  }
  
  ClassificationExample sample() const
  {
    DenseVectorPtr x = sampleVectorGaussian(numFeatures);
 //   std::cout << "BestScore: " << bestScore << " y = " << y << std::endl;
    return ClassificationExample(x, getClass(x));
  }
  
  size_t getClass(FeatureGeneratorPtr x) const
  {
    double bestScore = -DBL_MAX;
    size_t y = 0;
    //std::cout << "X dictionary: " << x->getDictionary().getName() << std::endl;
    for (size_t i = 0; i < parameters.size(); ++i)
    {
      //std::cout << "params i dictionary: " << parameters[i]->getDictionary().getName() << std::endl;
      double score = parameters[i]->dotProduct(x);
      if (score > bestScore)
        bestScore = score, y = i;
    }
    return y;
  }
  
  static FeatureDictionaryPtr getDictionary()
  {
    static FeatureDictionaryPtr dictionary = new FeatureDictionary("SyntheticLinearMultiClassGenerator");
    return dictionary;
  }
  
private:
  std::vector<DenseVectorPtr> parameters;
  size_t numFeatures;
  
  static DenseVectorPtr sampleVectorGaussian(size_t numFeatures)
  {
    DenseVectorPtr res = new DenseVector(getDictionary(), numFeatures);
    for (size_t i = 0; i < numFeatures; ++i)
      res->set(i, Random::getInstance().sampleDoubleFromGaussian());
    return res;
  }
};

class SyntheticDataGenerator
{
public:
  SyntheticDataGenerator(size_t numFeaturesInBranch, size_t numFolds, size_t numFeaturesPerFold, size_t numClasses)
    : branchGenerator(numFeaturesInBranch, numFolds), foldGenerators(numFolds, SyntheticLinearMultiClassGenerator(numFeaturesPerFold, numClasses))
    {}

  ClassificationExample sample() const
  {
    DenseVectorPtr x = new DenseVector(getDictionary(), 0, 1 + foldGenerators.size());
    ClassificationExample branch = branchGenerator.sample();
    x->setSubVector(0, branch.getInput());
  //  std::cout << "BRANCH output: " << branch.getOutput() << std::endl;
    size_t y = 0;
    for (size_t i = 0; i < foldGenerators.size(); ++i)
    {
      ClassificationExample fold = foldGenerators[i].sample();
      x->setSubVector(i + 1, fold.getInput());
      if (i == branch.getOutput())
        y = fold.getOutput();
    }
    return ClassificationExample(x, y);
  }

  static FeatureDictionaryPtr getDictionary()
  {
    static FeatureDictionaryPtr dictionary = new FeatureDictionary("SyntheticDataGenerator");
    return dictionary;
  }

  SyntheticLinearMultiClassGenerator branchGenerator;
  std::vector<SyntheticLinearMultiClassGenerator> foldGenerators;
};

}; /* namespace lcpp */

#endif // !SYNTHETIC_DATA_GENERATOR_H_
