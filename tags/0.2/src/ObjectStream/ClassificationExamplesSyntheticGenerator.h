/*-----------------------------------------.---------------------------------.
| Filename: ClassificationExamplesSynth...h| Synthetic generator for         |
| Author  : Francis Maes                   |   classification examples       |
| Started : 22/06/2009 18:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_STREAM_CLASSIFICATION_EXAMPLES_SYNTHETIC_GENERATOR_H_
# define LBCPP_OBJECT_STREAM_CLASSIFICATION_EXAMPLES_SYNTHETIC_GENERATOR_H_

# include <lbcpp/Object/ObjectStream.h>
# include <cfloat>

namespace lbcpp
{

class ClassificationExamplesSyntheticGenerator : public ObjectStream
{
public:
  ClassificationExamplesSyntheticGenerator(size_t numFeatures, size_t numClasses)
    : parameters(numClasses), numFeatures(numFeatures)
  {
    for (size_t i = 0; i < parameters.size(); ++i)
      parameters[i] = sampleVectorGaussian(numFeatures);
  }
  
  virtual String getContentClassName() const
    {return "ClassificationExample";}

  virtual ObjectPtr next()
  {
    DenseVectorPtr x = sampleVectorGaussian(numFeatures);
    return new ClassificationExample(x, getClass(x));
  }
  
  size_t getClass(FeatureGeneratorPtr x) const
  {
    double bestScore = -DBL_MAX;
    size_t y = 0;
    for (size_t i = 0; i < parameters.size(); ++i)
    {
      double score = parameters[i]->dotProduct(x);
      if (score > bestScore)
        bestScore = score, y = i;
    }
    return y;
  }
  
  static FeatureDictionaryPtr getDictionary()
  {
    static FeatureDictionaryPtr dictionary;
    if (!dictionary)
      dictionary = FeatureDictionaryManager::getInstance().getOrCreateRootDictionary(T("SyntheticLinearMultiClassGenerator"), true, true);
    return dictionary;
  }
  
private:
  std::vector<DenseVectorPtr> parameters;
  size_t numFeatures;
  
  static DenseVectorPtr sampleVectorGaussian(size_t numFeatures)
  {
    DenseVectorPtr res = new DenseVector(getDictionary(), numFeatures);
    for (size_t i = 0; i < numFeatures; ++i)
      res->set(i, RandomGenerator::getInstance().sampleDoubleFromGaussian());
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_STREAM_CLASSIFICATION_EXAMPLES_SYNTHETIC_GENERATOR_H_
