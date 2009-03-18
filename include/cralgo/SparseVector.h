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

namespace cralgo
{

class SparseVector : public FeatureGeneratorDefaultImplementations<SparseVector, DoubleVector>
{
public:
  typedef FeatureGeneratorDefaultImplementations<SparseVector, DoubleVector> BaseClass;
  
  SparseVector(const SparseVector& otherVector);
  SparseVector(FeatureDictionaryPtr dictionary);
  SparseVector();
  
  virtual ~SparseVector()
    {clear();}
  
  virtual void clear();
  SparseVector& operator =(const SparseVector& otherVector);
  
  /*
  ** Features
  */
  bool hasValues() const
    {return values.size() > 0;}
    
  size_t getNumValues() const
    {return values.size();}
  
  void set(size_t index, double value);
  void set(const std::string& name, double value);
  void set(const std::vector<std::string>& path, double value);

  double get(size_t index) const;
  double& get(size_t index);
  
  /*
  ** Sub Vectors
  */
  bool hasSubVectors() const
    {return subVectors.size() > 0;}
    
  size_t getNumSubVectors() const
    {return subVectors.size();}

  SparseVectorPtr& getSubVector(size_t index);

  void setSubVector(size_t index, SparseVectorPtr subVector)
    {getSubVector(index) = subVector;}
  
  /*
  ** Operations
  */
  size_t size() const;
  void multiplyByScalar(double scalar);

  void addWeighted(const FeatureGeneratorPtr featureGenerator, double weight)
    {featureGenerator->addWeightedTo(SparseVectorPtr(this), weight, dictionary);}

  /*
  ** Static FeatureGenerator
  */
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionaryPtr featureDictionary) const;

  /*
  ** FeatureGenerator
  */
  virtual std::string getName() const
    {return "SparseVector";}
    
  virtual FeatureDictionaryPtr getDictionary() const;

  virtual SparseVectorPtr toSparseVector(FeatureDictionaryPtr dictionary)
    {/* todo: check dictionary */ return SparseVectorPtr(this);}

protected:
  /*
  ** Object
  */
  virtual bool load(std::istream& istr);
  virtual void save(std::ostream& ostr) const;

private:
  typedef std::vector<std::pair<size_t, double> > FeatureVector;
  typedef std::vector<std::pair<size_t, SparseVectorPtr> > SubVectorVector;
  
  FeatureVector   values;
  SubVectorVector subVectors;
};

}; /* namespace cralgo */

#endif // !CRALGO_SPARSE_VECTOR_H_

