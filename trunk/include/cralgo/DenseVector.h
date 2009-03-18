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

class DenseVector : public FeatureGeneratorDefaultImplementations<DenseVector, DoubleVector>
{
public:
  typedef FeatureGeneratorDefaultImplementations<DenseVector, DoubleVector> BaseClass;

  DenseVector(const DenseVector& otherVector)
    : BaseClass(otherVector.dictionary), values(otherVector.values), subVectors(otherVector.subVectors) {}

  DenseVector(FeatureDictionaryPtr dictionary, size_t initialNumValues = 0, size_t initialNumSubVectors = 0)
    : BaseClass(dictionary)
  {
    initialize(initialNumValues, initialNumSubVectors);
  }
    
  DenseVector(size_t initialNumValues = 0, size_t initialNumSubVectors = 0)
    {initialize(initialNumValues, initialNumSubVectors);}
  
  DenseVector(const std::vector<double>& values)
    : values(values) {}
  
  virtual ~DenseVector()
    {clear();}
  
  virtual void clear()
    {values.clear(); subVectors.clear(); dictionary = FeatureDictionaryPtr();}
    
  DenseVector& operator =(const DenseVector& otherVector);

  /*
  ** Features
  */
  bool hasValues() const
    {return values.size() > 0;}
    
  const std::vector<double>& getValues() const
    {return values;}
    
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
  {
    ensureSize(subVectors, index + 1, DenseVectorPtr());
    subVectors[index] = subVector;
    if (dictionary)
    {
      // ensure 
      dictionary->getSubDictionary(index, subVector->dictionary);
    }
  }
  
  /*
  ** Operations
  */
  size_t size() const;
  virtual double dotProduct(const FeatureGeneratorPtr featureGenerator) const;
  void multiplyByScalar(double scalar);
  
  void addWeighted(const LazyVectorPtr lazyVector, double weight);
    
  void add(const FeatureGeneratorPtr featureGenerator)
    {featureGenerator->addTo(DenseVectorPtr(this));}
  void substract(const FeatureGeneratorPtr featureGenerator)
    {featureGenerator->substractFrom(DenseVectorPtr(this));}
  void addWeighted(const FeatureGeneratorPtr featureGenerator, double weight)
    {featureGenerator->addWeightedTo(DenseVectorPtr(this), weight, dictionary);}

  /*
  ** Static FeatureGenerator
  */
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionaryPtr featureDictionary) const;

  /*
  ** FeatureGenerator
  */
  virtual std::string getName() const
    {return "DenseVector";}
    
  virtual FeatureDictionaryPtr getDictionary() const;
  
  virtual DenseVectorPtr toDenseVector(FeatureDictionaryPtr dictionary)
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

