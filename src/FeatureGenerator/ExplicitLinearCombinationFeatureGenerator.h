/*-----------------------------------------.---------------------------------.
| Filename: ExplicitLinearCombinationFe...h| Linear combination of           |
| Author  : Francis Maes                   |      feature generators         |
| Started : 22/03/2009 17:57               |  stored with a std::vector      |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_EXPLICIT_LINEAR_COMBINATION_H_
# define LBCPP_FEATURE_GENERATOR_EXPLICIT_LINEAR_COMBINATION_H_

# include <lbcpp/FeatureGenerator.h>
# include <lbcpp/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp>

namespace lbcpp
{

class ExplicitLinearCombinationFeatureGenerator
  : public FeatureGeneratorDefaultImplementations<ExplicitLinearCombinationFeatureGenerator, LazyFeatureVector>
{
public:
  typedef FeatureGeneratorDefaultImplementations<ExplicitLinearCombinationFeatureGenerator, LazyFeatureVector> BaseClass;
  
  typedef std::vector< std::pair<FeatureGeneratorPtr, double> > LinearCombinationVector;
  
  ExplicitLinearCombinationFeatureGenerator(LinearCombinationVector* newTerms)
    : terms(newTerms)
  {
    assert(terms && terms->size());
# ifdef NDEBUG
    setDictionary((*terms)[0].first->getDictionary());
# else
    for (size_t i = 0; i < terms->size(); ++i)
    {
      assert((*terms)[i].first);
      ensureDictionary((*terms)[i].first->getDictionary());
    }
# endif
  }
    
  ~ExplicitLinearCombinationFeatureGenerator()
    {delete terms;}

  /*
  ** Accessors
  */
  size_t getNumTerms() const
    {return terms->size();}
    
  FeatureGeneratorPtr getFeatureGenerator(size_t i) const
    {assert(i < terms->size()); return (*terms)[i].first;}

  double getWeight(size_t i) const
    {assert(i < terms->size()); return (*terms)[i].second;}

  /*
  ** EditableFeatureGenerator
  */
  virtual void clear()
    {terms->clear();}


  /*
  ** LazyFeatureVector
  */
  virtual bool isDense() const
  {
    if (terms->size() > 20)
      return true; // many terms: use dense vector
    for (size_t i = 0; i < terms->size(); ++i)
      if ((*terms)[i].first->isDense())
        return true;
    return false;
  }

  virtual FeatureVectorPtr computeVector() const
  {
    if (isDense())
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
    }
  }

private:
  LinearCombinationVector* terms;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_EXPLICIT_LINEAR_COMBINATION_H_
