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

class SyntheticDataGenerator : public ObjectStream
{
public:
  SyntheticDataGenerator(size_t numFeaturesInBranch, size_t numFolds, size_t numFeaturesPerFold, size_t numClasses)
    : branchGenerator(classificationExamplesSyntheticGenerator(numFeaturesInBranch, numFolds)), foldGenerators(numFolds)
  {
    for (size_t i = 0; i < foldGenerators.size(); ++i)
      foldGenerators[i] = classificationExamplesSyntheticGenerator(numFeaturesPerFold, numClasses);
  }

  virtual String getContentClassName() const
    {return T("ClassificationExample");}

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

  ObjectStreamPtr branchGenerator;
  std::vector<ObjectStreamPtr> foldGenerators;
};

}; /* namespace lbcpp */

#endif // !SYNTHETIC_DATA_GENERATOR_H_
