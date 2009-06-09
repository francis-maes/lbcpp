/*-----------------------------------------.---------------------------------.
| Filename: DenseVector.h                  | Composite Dense Vectors         |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 18:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DENSE_VECTOR_H_
# define LBCPP_DENSE_VECTOR_H_

# include "EditableFeatureGenerator.h"

namespace lbcpp
{

class DenseVector : public FeatureGeneratorDefaultImplementations<DenseVector, FeatureVector>
{
public:
  typedef FeatureGeneratorDefaultImplementations<DenseVector, FeatureVector> BaseClass;

  DenseVector(const DenseVector& otherVector);
  DenseVector(FeatureDictionaryPtr dictionary, const std::vector<double>& values);
  DenseVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr(), size_t initialNumValues = 0, size_t initialNumSubVectors = 0);
  
  virtual ~DenseVector()
    {clear();}
  
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
  void initializeRandomly(double mean = 0.0, double standardDeviation = 1.0);
  
  void addWeighted(const DenseVectorPtr otherVector, double weight);
  
  void addWeighted(const FeatureGeneratorPtr featureGenerator, double weight)
  {
    const DenseVectorPtr dense = featureGenerator.dynamicCast<DenseVector>();
    if (dense)
      addWeighted(dense, weight);
    else
      featureGenerator->addWeightedTo(DenseVectorPtr(this), weight);
  }

  void add(const DenseVectorPtr otherVector);      
  void add(const FeatureGeneratorPtr featureGenerator)
    {featureGenerator->addTo(DenseVectorPtr(this));}
  void substract(const FeatureGeneratorPtr featureGenerator)
    {featureGenerator->substractFrom(DenseVectorPtr(this));}

  /*
  ** EditableFeatureGenerator
  */
  virtual void clear()
    {values.clear(); subVectors.clear(); dictionary = FeatureDictionaryPtr();}

  /*
  ** Static FeatureGenerator
  */
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor) const;

  /*
  ** FeatureGenerator
  */
  virtual FeatureDictionaryPtr getDictionary() const;

  virtual bool isDense() const
    {return true;}
  
  virtual DenseVectorPtr toDenseVector() const
    {return DenseVectorPtr(const_cast<DenseVector* >(this));}

  virtual size_t getNumSubGenerators() const
    {return subVectors.size();}

  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const
    {assert(num < subVectors.size()); return subVectors[num];}

  virtual size_t getSubGeneratorIndex(size_t num) const
    {assert(num < subVectors.size()); return num;}

  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const
    {return index < subVectors.size() ? (FeatureGeneratorPtr)subVectors[index] : FeatureGeneratorPtr();}

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
};

}; /* namespace lbcpp */

#endif // !LBCPP_DENSE_VECTOR_H_

