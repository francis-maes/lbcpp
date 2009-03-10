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
    : values(otherVector.values), subVectors(otherVector.subVectors) {dictionary = otherVector.dictionary;}

  DenseVector(FeatureDictionary& dictionary, size_t initialNumValues = 0, size_t initialNumSubVectors = 0)
  {
    this->dictionary = &dictionary;
    initialize(initialNumValues, initialNumSubVectors);
  }
    
  DenseVector(size_t initialNumValues = 0, size_t initialNumSubVectors = 0)
  {
    dictionary = NULL;
    initialize(initialNumValues, initialNumSubVectors);
  }
  
  virtual ~DenseVector()
    {clear();}
  
  virtual void clear()
    {values.clear(); subVectors.clear(); dictionary = NULL;}
    
  DenseVector& operator =(const DenseVector& otherVector);

  /*
  ** Features
  */
  bool hasValues() const
    {return values.size() > 0;}
    
  size_t getNumValues() const
    {return values.size();}
  
  void set(size_t index, double value)
    {ensureSize(values, index + 1, 0.0); values[index] = value;}
    
  const double& get(size_t index) const
    {assert(index < values.size()); return values[index];}

  double& get(size_t index)
    {ensureSize(values, index + 1, 0.0); return values[index];}
    
  int findIndexOfMaximumValue() const;
  double findMaximumValue() const;
  
  double computeLogSumOfExponentials() const;   // return log(sum_i exp(x_i)), avoiding numerical errors with too big exponentials

  /*
  ** Sub Vectors
  */
  bool hasSubVectors() const
    {return subVectors.size() > 0;}
    
  size_t getNumSubVectors() const
    {return subVectors.size();}

  DenseVectorPtr& getSubVector(size_t index)
    {ensureSize(subVectors, index + 1, DenseVectorPtr()); return subVectors[index];}
    
  void setSubVector(size_t index, DenseVectorPtr subVector)
    {ensureSize(subVectors, index + 1, DenseVectorPtr()); subVectors[index] = subVector;}
  
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
  
  virtual DenseVectorPtr toDenseVector(FeatureDictionary* dictionary)
    {/* todo: check dictionary */ return DenseVectorPtr(this);}

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
  std::vector<double> values;
  std::vector<DenseVectorPtr> subVectors;

  double denseDotProduct(const DenseVectorPtr otherVector) const;
  void initialize(size_t initialNumValues, size_t initialNumSubVectors);
};

}; /* namespace cralgo */

#endif // !CRALGO_DENSE_VECTOR_H_

