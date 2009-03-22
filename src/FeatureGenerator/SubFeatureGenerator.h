/*-----------------------------------------.---------------------------------.
| Filename: SubFeatureGenerator.h          | Sub Feature-Generator           |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2009 17:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_FEATURE_GENERATOR_SUB_H_
# define CRALGO_FEATURE_GENERATOR_SUB_H_

# include <cralgo/FeatureGenerator.h>
# include <cralgo/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp>

namespace cralgo
{

class SubFeatureGenerator :
  public FeatureGeneratorDefaultImplementations<SubFeatureGenerator, EditableFeatureGenerator>
{
public:
  typedef FeatureGeneratorDefaultImplementations<SubFeatureGenerator, EditableFeatureGenerator> BaseClass;
  
  // fixme: dictionary
  SubFeatureGenerator(size_t index, FeatureGeneratorPtr featureGenerator)
    : index(index), featureGenerator(featureGenerator) {}
  
  /*
  ** Accessors
  */
  bool exists() const
    {return index != (size_t)-1;}
  
  size_t getIndex() const
    {return index;}
    
  FeatureGeneratorPtr getFeatureGenerator() const
    {return featureGenerator;}

  /*
  ** EditableFeatureGenerator
  */
  virtual void clear()
    {index = (size_t)-1; featureGenerator = FeatureGeneratorPtr();}
  
  /*
  ** FeatureGeneraor
  */
  virtual bool isDense() const
    {return featureGenerator->isDense();}

  virtual size_t getNumSubGenerators() const
    {return 1;}
    
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const
    {return num == 0 ? featureGenerator : FeatureGeneratorPtr();}

  virtual size_t getSubGeneratorIndex(size_t num) const
    {return num == 0 ? index : (size_t)-1;}

  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const
    {return index == this->index ? featureGenerator : FeatureGeneratorPtr();}

  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionaryPtr featureDictionary) const
    {assert(false);}

private:
  size_t index;
  FeatureGeneratorPtr featureGenerator;
};

}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_GENERATOR_SUB_H_
