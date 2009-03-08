/*-----------------------------------------.---------------------------------.
| Filename: DenseVector.h                  | Composite Dense Vectors         |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 18:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_DENSE_VECTOR_H_
# define CRALGO_DENSE_VECTOR_H_

# include "DoubleVector.h"

namespace cralgo
{

class DenseVector;
typedef ReferenceCountedObjectPtr<DenseVector> DenseVectorPtr;
class LazyVector;
typedef ReferenceCountedObjectPtr<LazyVector> LazyVectorPtr;

class DenseVector : public FeatureGeneratorDefaultImplementations<DenseVector, DoubleVector>
{
public:
  DenseVector(const DenseVector& otherVector)
    : features(otherVector.features), subVectors(otherVector.subVectors) {dictionary = otherVector.dictionary;}

  DenseVector(FeatureDictionary& dictionary)
    {this->dictionary = &dictionary;}
    
  DenseVector()
    {dictionary = NULL;}
  
  virtual ~DenseVector()
    {clear();}
  
  void clear()
    {features.clear(); subVectors.clear(); dictionary = NULL;}
    
  DenseVector& operator =(const DenseVector& otherVector);

  /*
  ** Features
  */
  bool hasFeatures() const
    {return features.size() > 0;}
    
  size_t getNumFeatures() const
    {return features.size();}
  
  void set(size_t index, double value)
    {ensureSize(features, index + 1, 0.0); features[index] = value;}
    
  double& get(size_t index)
    {ensureSize(features, index + 1, 0.0); return features[index];}

  /*
  ** Sub Vectors
  */
  bool hasSubVectors() const
    {return subVectors.size() > 0;}
    
  size_t getNumSubVectors() const
    {return subVectors.size();}

  DenseVectorPtr& getSubVector(size_t index)
    {ensureSize(subVectors, index + 1, DenseVectorPtr()); return subVectors[index];}
  
  /*
  ** Operations
  */
  size_t size() const;
  virtual double dotProduct(const FeatureGeneratorPtr featureGenerator) const;
  void multiplyByScalar(double scalar);
  
  void addWeighted(const LazyVectorPtr lazyVector, double weight);
    
  void addWeighted(const FeatureGeneratorPtr featureGenerator, double weight)
    {featureGenerator->addWeightedTo(DenseVectorPtr(this), weight);}

  /*
  ** Static FeatureGenerator
  */
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionary& featureDictionary) const;

  /*
  ** FeatureGenerator
  */
  virtual std::string getName() const
    {return "DenseVector";}
    
  virtual FeatureDictionary& getDefaultDictionary() const
    {static FeatureDictionary defaultDictionary("DenseVector"); return dictionary ? *dictionary : defaultDictionary;}
  
protected:
  /*
  ** Object
  */
  virtual bool load(std::istream& istr);
  virtual void save(std::ostream& ostr) const;

protected:
  template<class VectorType, class ContentType>
  void ensureSize(VectorType& vector, size_t minimumSize, const ContentType& defaultValue)
  {
    if (vector.size() < minimumSize)
    {
      if (vector.size() && minimumSize > 100000 * vector.size())
        std::cerr << "Warning: really big feature index " << minimumSize - 1
                  << "! Use SparseVectors or change your feature indices." << std::endl;
      vector.resize(minimumSize, defaultValue);
    }
  }
    
private:
  std::vector<double> features;
  std::vector<DenseVectorPtr> subVectors;

  double denseDotProduct(const DenseVectorPtr otherVector) const;
};

}; /* namespace cralgo */

#endif // !CRALGO_DENSE_VECTOR_H_

