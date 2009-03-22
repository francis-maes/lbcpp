/*-----------------------------------------.---------------------------------.
| Filename: LinearCombinationFeatureGen...h| Linear combination of           |
| Author  : Francis Maes                   |     feature generators          |
| Started : 22/03/2009 17:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_FEATURE_GENERATOR_LINEAR_COMBINATION_H_
# define CRALGO_FEATURE_GENERATOR_LINEAR_COMBINATION_H_

# include <cralgo/FeatureGenerator.h>
# include <cralgo/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp>

namespace cralgo
{

class LinearCombinationFeatureGenerator
  : public FeatureGeneratorDefaultImplementations<LinearCombinationFeatureGenerator, LazyFeatureVector>
{
public:
  typedef FeatureGeneratorDefaultImplementations<LinearCombinationFeatureGenerator, LazyFeatureVector> BaseClass;
  
  LinearCombinationFeatureGenerator(FeatureGeneratorPtr compositeFeatureGenerator, DenseVectorPtr weights)
    : compositeFeatureGenerator(compositeFeatureGenerator), weights(weights) {}
  
  virtual void clear()
    {compositeFeatureGenerator = FeatureGeneratorPtr(); weights = DenseVectorPtr();}
  
  // fixme: dictionary, static_visitor
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionaryPtr featureDictionary) const
    {assert(false);} // FIXME: not implemented yet
  
  /*
  ** LazyFeatureVector
  */
  virtual bool isDense() const
    {return compositeFeatureGenerator->isDense() || weights->size() > 20;}

  virtual FeatureVectorPtr computeVector() const
  {
    assert(false); // FIXME: not implemented yet
    return FeatureVectorPtr();
/*    if (isDense())
    {
      DenseVectorPtr res = new DenseVector(getDictionary());
      for (size_t i = 0; i < terms->size(); ++i)
      {
        std::pair<FeatureGeneratorPtr, double>& p = (*terms)[i];
        if (p.first && p.second != 0.0)
          res->addWeighted(p.first, p.second);
      }
      return res;
    }
    else
    {
      SparseVectorPtr res = new SparseVector(getDictionary());
      for (size_t i = 0; i < terms->size(); ++i)
      {
        std::pair<FeatureGeneratorPtr, double>& p = (*terms)[i];
        if (p.first && p.second != 0.0)
          res->addWeighted(p.first, p.second);
      }      
      return res;
    }*/
  }
    
private:
  FeatureGeneratorPtr compositeFeatureGenerator;
  DenseVectorPtr weights;
};

}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_GENERATOR_LINEAR_COMBINATION_H_
