/*-----------------------------------------.---------------------------------.
| Filename: SyntheticDataGenerator.h       | Synthetic Data Generator        |
| Author  : Francis Maes                   |                                 |
| Started : 29/03/2009 22:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef SYNTHETIC_DATA_GENERATOR_H_
# define SYNTHETIC_DATA_GENERATOR_H_

#include <lbcpp/lbcpp.h>

namespace lbcpp
{

class SyntheticLinearMultiClassGenerator : public ObjectStream
{
public:
  SyntheticLinearMultiClassGenerator(size_t numFeatures, size_t numClasses)
    : parameters(numClasses), numFeatures(numFeatures)
  {
    for (size_t i = 0; i < parameters.size(); ++i)
      parameters[i] = sampleVectorGaussian(numFeatures);
  }
  
  virtual std::string getContentClassName() const
    {return "ClassificationExample";}

  virtual ObjectPtr next()
  {
    DenseVectorPtr x = sampleVectorGaussian(numFeatures);
 //   std::cout << "BestScore: " << bestScore << " y = " << y << std::endl;
    return new ClassificationExample(x, getClass(x));
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

typedef ReferenceCountedObjectPtr<SyntheticLinearMultiClassGenerator> SyntheticLinearMultiClassGeneratorPtr;

class SyntheticDataGenerator : public ObjectStream
{
public:
  SyntheticDataGenerator(size_t numFeaturesInBranch, size_t numFolds, size_t numFeaturesPerFold, size_t numClasses)
    : branchGenerator(new SyntheticLinearMultiClassGenerator(numFeaturesInBranch, numFolds)), foldGenerators(numFolds)
  {
    for (size_t i = 0; i < foldGenerators.size(); ++i)
      foldGenerators[i] = new SyntheticLinearMultiClassGenerator(numFeaturesPerFold, numClasses);
  }

  virtual std::string getContentClassName() const
    {return "ClassificationExample";}

  virtual ObjectPtr next()
  {
    DenseVectorPtr x = new DenseVector(getDictionary(), 0, 1 + foldGenerators.size());
    ClassificationExamplePtr branch = branchGenerator->next();
    x->setSubVector(0, branch->getInput());
  //  std::cout << "BRANCH output: " << branch.getOutput() << std::endl;
    size_t y = 0;
    for (size_t i = 0; i < foldGenerators.size(); ++i)
    {
      ClassificationExamplePtr fold = foldGenerators[i]->next();
      x->setSubVector(i + 1, fold->getInput());
      if (i == branch->getOutput())
        y = fold->getOutput();
    }
    return new ClassificationExample(x, y);
  }

  static FeatureDictionaryPtr getDictionary()
  {
    static FeatureDictionaryPtr dictionary = new FeatureDictionary("SyntheticDataGenerator");
    return dictionary;
  }

  SyntheticLinearMultiClassGeneratorPtr branchGenerator;
  std::vector<SyntheticLinearMultiClassGeneratorPtr> foldGenerators;
};

}; /* namespace lbcpp */

#endif // !SYNTHETIC_DATA_GENERATOR_H_
