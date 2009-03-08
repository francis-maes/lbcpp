/*-----------------------------------------.---------------------------------.
| Filename: SparseVector.h                 | Sparse composite vectors        |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 18:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_SPARSE_VECTOR_H_
# define CRALGO_SPARSE_VECTOR_H_

# include "DoubleVector.h"
# include <boost/enable_shared_from_this.hpp>

namespace cralgo
{

class SparseVector;
typedef boost::shared_ptr<SparseVector> SparseVectorPtr;

class SparseVector : public FeatureGeneratorDefaultImplementations<SparseVector, DoubleVector>,
    public boost::enable_shared_from_this<SparseVector>
{
public:
  SparseVector(const SparseVector& otherVector);
  SparseVector(FeatureDictionary& dictionary);
  SparseVector();
  
  virtual ~SparseVector()
    {clear();}
  
  void clear();
  SparseVector& operator =(const SparseVector& otherVector);

  /*
  ** Shared Pointer to this
  */
  SparseVectorPtr getSharedPointer()
    {return shared_from_this();}
  
  /*
  ** Features
  */
  bool hasFeatures() const
    {return features.size() > 0;}
    
  size_t getNumFeatures() const
    {return features.size();}
  
  void set(size_t index, double value);
  void set(const std::string& name, double value);
  void set(const std::vector<std::string>& path, double value);
  
  /*
  ** Sub Vectors
  */
  bool hasSubVectors() const
    {return subVectors.size() > 0;}
    
  size_t getNumSubVectors() const
    {return subVectors.size();}

  SparseVectorPtr& getSubVector(size_t index);
  
  /*
  ** Operations
  */
  size_t size() const;

  /*
  ** Static FeatureGenerator
  */
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionary& featureDictionary) const;

  /*
  ** FeatureGenerator
  */
  virtual std::string getName() const
    {return "SparseVector";}
    
  virtual FeatureDictionary& getDefaultDictionary() const
    {static FeatureDictionary defaultDictionary("SparseVector"); return dictionary ? *dictionary : defaultDictionary;}

  virtual SparseVectorPtr createSparseVector(FeatureDictionary* dictionary)
    {/* todo: check dictionary */ return getSharedPointer();}

protected:
  /*
  ** Object
  */
  virtual bool load(std::istream& istr);
  virtual void save(std::ostream& ostr) const;

private:
  typedef std::vector<std::pair<size_t, double> > FeatureVector;
  typedef std::vector<std::pair<size_t, SparseVectorPtr> > SubVectorVector;
  
  FeatureVector   features;
  SubVectorVector subVectors;
};

}; /* namespace cralgo */

#endif // !CRALGO_SPARSE_VECTOR_H_

