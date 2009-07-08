/*-----------------------------------------.---------------------------------.
| Filename: DenseVector.h                  | Composite Dense Vectors         |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 18:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   DenseVector.h
**@author Francis MAES
**@date   Tue Jun 16 08:30:42 2009
**
**@brief  Composite dense vector declarations.
**
**
*/

#ifndef LBCPP_DENSE_VECTOR_H_
# define LBCPP_DENSE_VECTOR_H_

# include "EditableFeatureGenerator.h"

namespace lbcpp
{

/*!
** @class DenseVector
** @brief Dense vector are vectors where every features are specify whatever the values are.
*/

class DenseVector : public FeatureGeneratorDefaultImplementations<DenseVector, FeatureVector>
{
public:
  typedef FeatureGeneratorDefaultImplementations<DenseVector, FeatureVector> BaseClass;

  /*!
  ** Copy constructor.
  **
  ** @param otherVector : original dense vector.
  **
  ** @return a DenseVector instance.
  */
  DenseVector(const DenseVector& otherVector);

  /*!
  ** Constructor.
  **
  ** @param dictionary : feature dictionary.
  ** @param values : features values.
  **
  ** @return a DenseVector instance.
  */
  DenseVector(FeatureDictionaryPtr dictionary, const std::vector<double>& values);

  /*!
  ** Constructor.
  **
  ** @param dictionary : feature dictionary.
  ** @param initialNumValues :
  ** @param initialNumSubVectors :
  **
  ** @return a DenseVector instance.
  */
  DenseVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr(), size_t initialNumValues = 0, size_t initialNumSubVectors = 0);

  /*!
  ** Destructor.
  */
  virtual ~DenseVector()
    {clear();}

  /*!
  ** = operator.
  **
  ** @param otherVector : original dense vector.
  **
  ** @return a DenseVector instance.
  */
  DenseVector& operator =(const DenseVector& otherVector);

  /*
  ** Features
  */
  /*!
  ** Checks if some features are setted.
  **
  ** @return True if any feature is setted.
  */
  bool hasValues() const
    {return values.size() > 0;}

  /*!
  ** Values getter.
  **
  ** @return a vector of values.
  */
  const std::vector<double>& getValues() const
    {return values;}

  /*!
  ** Returns the number of values.
  **
  ** @return the number of values.
  */
  size_t getNumValues() const
    {return values.size();}

  /*!
  ** Feature value setter.
  **
  ** @param index : feature index.
  ** @param value : feature value.
  */
  void set(size_t index, double value)
    {ensureSize(values, index + 1, 0.0); values[index] = value;}

  /*!
  ** Feature value getter.
  **
  ** @param index : feature index.
  **
  ** @return feature value at the index @a index.
  */
  const double& get(size_t index) const
    {assert(index < values.size()); return values[index];}

  /*!
  ** Feature value getter.
  **
  ** @param index : feature index.
  **
  ** @return feature value at the index @a index.
  */
  double& get(size_t index)
    {ensureSize(values, index + 1, 0.0); return values[index];}

  /*!
  ** Returns the index of the maximum value.
  **
  ** @return the index of the maximum value.
  */
  int findIndexOfMaximumValue() const;

  /*!
  ** Returns the maximum value.
  **
  ** @return the maximum value.
  */
  double findMaximumValue() const;

  /*!
  ** Computes the logarithmic sum of exponentials.
  **
  ** @return log(sum_i exp(x_i)), avoiding numerical errors with too
  ** big exponentials.
  */
  double computeLogSumOfExponentials() const;

  /*
  ** Sub Vectors
  */
  /*!
  ** Checks if any subvector is setted.
  **
  ** @return True if any subvector is setted.
  */
  bool hasSubVectors() const
    {return subVectors.size() > 0;}

  /*!
  ** Returns the number of subvectors.
  **
  ** @return the number of subvectors.
  */
  size_t getNumSubVectors() const
    {return subVectors.size();}

  /*!
  ** Subvector geter.
  **
  ** @param index : subvector index.
  **
  ** @return a DenseVector reference.
  */
  DenseVectorPtr& getSubVector(size_t index)
    {ensureSize(subVectors, index + 1, DenseVectorPtr()); return subVectors[index];}

  /*!
  ** Subvector setter.
  **
  ** @param index : subvector index.
  ** @param subVector : subvector value.
  */
  void setSubVector(size_t index, DenseVectorPtr subVector)
  {
    ensureSize(subVectors, index + 1, DenseVectorPtr());
    subVectors[index] = subVector;
    if (dictionary)
      dictionary->ensureSubDictionary(index, subVector->dictionary);
  }

  /*
  ** Operations
  */
  /*!
  ** Size getter (number of values, recursive).
  **
  ** @return the number of values.
  */
  size_t size() const;

  /*!
  ** Scalar product.
  **
  ** @param featureGenerator : right operand.
  **
  ** @return scalar product result.
  */
  virtual double dotProduct(const FeatureGeneratorPtr featureGenerator) const;

  /*!
  ** Multiplication by a scalar value.
  **
  ** @param scalar : scalar value.
  */
  void multiplyByScalar(double scalar);

  /*!
  ** Randomly initializes the dense vector.
  **
  ** @param mean : mean.
  ** @param standardDeviation : standard deviation.
  */
  void initializeRandomly(double mean = 0.0, double standardDeviation = 1.0);

  /*!
  ** Adds a weighted dense vector.
  **
  ** @param otherVector : dense vector.
  ** @param weight : scalar value.
  */
  void addWeighted(const DenseVectorPtr otherVector, double weight);

  /*!
  ** Adds a weighted feature generator (read only dense vector).
  **
  ** @param featureGenerator : feature generator.
  ** @param weight : scalar value.
  */
  void addWeighted(const FeatureGeneratorPtr featureGenerator, double weight)
  {
    const DenseVectorPtr dense = featureGenerator.dynamicCast<DenseVector>();
    if (dense)
      addWeighted(dense, weight);
    else
      featureGenerator->addWeightedTo(DenseVectorPtr(this), weight);
  }

  /*!
  ** Adds a dense vector.
  **
  ** @param otherVector : dense vector.
  */
  void add(const DenseVectorPtr otherVector);

  /*!
  ** Adds a feature generator (read only dense vector).
  **
  ** @param featureGenerator : feature generator.
  */
  void add(const FeatureGeneratorPtr featureGenerator)
    {featureGenerator->addTo(DenseVectorPtr(this));}

  /*!
  ** Substracts a feature generator (read only dense vector).
  **
  ** @param featureGenerator
  */
  void substract(const FeatureGeneratorPtr featureGenerator)
    {featureGenerator->substractFrom(DenseVectorPtr(this));}

  /*
  ** EditableFeatureGenerator
  */
  /*!
  ** Clears all (values, subvectors, dictionary).
  **
  */
  virtual void clear()
    {values.clear(); subVectors.clear(); dictionary = FeatureDictionaryPtr();}

  /*
  ** Static FeatureGenerator
  */
  /*!
  ** #FIXME
  **
  ** @param visitor
  */
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor) const;

  /*
  ** FeatureGenerator
  */
  /*!
  ** Feature dictionary getter.
  **
  ** @return a feature dictionary pointer.
  */
  virtual FeatureDictionaryPtr getDictionary() const;

  /*!
  ** Checks if the vector is dense.
  **
  ** @return True if the vector is dense.
  */
  virtual bool isDense() const
    {return true;}

  /*!
  ** Converts vector to dense vector.
  **
  ** @return a dense vector pointer.
  */
  virtual DenseVectorPtr toDenseVector() const
    {return DenseVectorPtr(const_cast<DenseVector* >(this));}

  /*!
  ** Returns the number of subvectors.
  **
  ** @return the number of subvectors
  */
  virtual size_t getNumSubGenerators() const
    {return subVectors.size();}

  /*!
  ** #FIXME
  **
  ** @param num
  **
  ** @return
  */
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const
    {assert(num < subVectors.size()); return subVectors[num];}

  /*!
  ** #FIXME
  **
  ** @param num
  **
  ** @return
  */
  virtual size_t getSubGeneratorIndex(size_t num) const
    {assert(num < subVectors.size()); return num;}

  /*!
  ** #FIXME
  **
  ** @param index
  **
  ** @return
  */
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const
    {return index < subVectors.size() ? (FeatureGeneratorPtr)subVectors[index] : FeatureGeneratorPtr();}

  /*
  ** Object
  */
  /*!
  ** Loads dense vector from a stream.
  **
  ** @param istr : input stream.
  **
  ** @return False if any error occurs.
  */
  virtual bool load(std::istream& istr);

  /*!
  ** Saves dense vector to a stream.
  **
  ** @param ostr : output stream.
  */
  virtual void save(std::ostream& ostr) const;

  /*!
  ** Clones dense vector.
  **
  ** @return
  */
  virtual ObjectPtr clone() const
    {return new DenseVector(*this);}

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

/*!
** Loads dense vector from file @a filename.
**
** @param filename : input file name.
**
** @return a new DenseVector instance.
*/
inline DenseVectorPtr loadDenseVector(const std::string& filename)
  {return Object::loadFromFileAndCast<DenseVector>(filename);}

}; /* namespace lbcpp */

#endif // !LBCPP_DENSE_VECTOR_H_

