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
**@brief  #FIXME: all
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
** @brief
*/

class DenseVector : public FeatureGeneratorDefaultImplementations<DenseVector, FeatureVector>
{
public:
  typedef FeatureGeneratorDefaultImplementations<DenseVector, FeatureVector> BaseClass;

  /*!
  **
  **
  ** @param otherVector
  **
  ** @return
  */
  DenseVector(const DenseVector& otherVector);
  /*!
  **
  **
  ** @param dictionary
  ** @param values
  **
  ** @return
  */
  DenseVector(FeatureDictionaryPtr dictionary, const std::vector<double>& values);
  /*!
  **
  **
  ** @param initialNumSubVectors
  **
  ** @return
  */
  DenseVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr(), size_t initialNumValues = 0, size_t initialNumSubVectors = 0);

  /*!
  **
  **
  **
  ** @return
  */
  virtual ~DenseVector()
    {clear();}

  /*!
  **
  **
  ** @param otherVector
  **
  ** @return
  */
  DenseVector& operator =(const DenseVector& otherVector);

  /*
  ** Features
  */
  /*!
  **
  **
  **
  ** @return
  */
  bool hasValues() const
    {return values.size() > 0;}

  /*!
  **
  **
  **
  ** @return
  */
  const std::vector<double>& getValues() const
    {return values;}

  /*!
  **
  **
  **
  ** @return
  */
  size_t getNumValues() const
    {return values.size();}

  /*!
  **
  **
  ** @param index
  ** @param value
  */
  void set(size_t index, double value)
    {ensureSize(values, index + 1, 0.0); values[index] = value;}

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  const double& get(size_t index) const
    {assert(index < values.size()); return values[index];}

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  double& get(size_t index)
    {ensureSize(values, index + 1, 0.0); return values[index];}

  /*!
  **
  **
  **
  ** @return
  */
  int findIndexOfMaximumValue() const;
  /*!
  **
  **
  **
  ** @return
  */
  double findMaximumValue() const;

  /*!
  **
  **
  **
  ** @return
  */
  double computeLogSumOfExponentials() const;   // return log(sum_i exp(x_i)), avoiding numerical errors with too big exponentials

  /*
  ** Sub Vectors
  */
  /*!
  **
  **
  **
  ** @return
  */
  bool hasSubVectors() const
    {return subVectors.size() > 0;}

  /*!
  **
  **
  **
  ** @return
  */
  size_t getNumSubVectors() const
    {return subVectors.size();}

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  DenseVectorPtr& getSubVector(size_t index)
    {ensureSize(subVectors, index + 1, DenseVectorPtr()); return subVectors[index];}

  /*!
  **
  **
  ** @param index
  ** @param subVector
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
  **
  **
  **
  ** @return
  */
  size_t size() const;

  /*!
  **
  **
  ** @param featureGenerator
  **
  ** @return
  */
  virtual double dotProduct(const FeatureGeneratorPtr featureGenerator) const;
  /*!
  **
  **
  ** @param scalar
  */
  void multiplyByScalar(double scalar);
  /*!
  **
  **
  ** @param mean
  ** @param standardDeviation
  */
  void initializeRandomly(double mean = 0.0, double standardDeviation = 1.0);

  /*!
  **
  **
  ** @param otherVector
  ** @param weight
  */
  void addWeighted(const DenseVectorPtr otherVector, double weight);

  /*!
  **
  **
  ** @param featureGenerator
  ** @param weight
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
  **
  **
  ** @param otherVector
  */
  void add(const DenseVectorPtr otherVector);
  /*!
  **
  **
  ** @param featureGenerator
  */
  void add(const FeatureGeneratorPtr featureGenerator)
    {featureGenerator->addTo(DenseVectorPtr(this));}
  /*!
  **
  **
  ** @param featureGenerator
  */
  void substract(const FeatureGeneratorPtr featureGenerator)
    {featureGenerator->substractFrom(DenseVectorPtr(this));}

  /*
  ** EditableFeatureGenerator
  */
  /*!
  **
  **
  */
  virtual void clear()
    {values.clear(); subVectors.clear(); dictionary = FeatureDictionaryPtr();}

  /*
  ** Static FeatureGenerator
  */
  /*!
  **
  **
  ** @param visitor
  */
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor) const;

  /*
  ** FeatureGenerator
  */
  /*!
  **
  **
  **
  ** @return
  */
  virtual FeatureDictionaryPtr getDictionary() const;

  /*!
  **
  **
  **
  ** @return
  */
  virtual bool isDense() const
    {return true;}

  /*!
  **
  **
  **
  ** @return
  */
  virtual DenseVectorPtr toDenseVector() const
    {return DenseVectorPtr(const_cast<DenseVector* >(this));}

  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t getNumSubGenerators() const
    {return subVectors.size();}

  /*!
  **
  **
  ** @param num
  **
  ** @return
  */
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const
    {assert(num < subVectors.size()); return subVectors[num];}

  /*!
  **
  **
  ** @param num
  **
  ** @return
  */
  virtual size_t getSubGeneratorIndex(size_t num) const
    {assert(num < subVectors.size()); return num;}

  /*!
  **
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
  **
  **
  ** @param istr
  **
  ** @return
  */
  virtual bool load(std::istream& istr);

  /*!
  **
  **
  ** @param ostr
  */
  virtual void save(std::ostream& ostr) const;

  /*!
  **
  **
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
**
**
** @param filename
**
** @return
*/
inline DenseVectorPtr loadDenseVector(const std::string& filename)
  {return Object::loadFromFileAndCast<DenseVector>(filename);}

}; /* namespace lbcpp */

#endif // !LBCPP_DENSE_VECTOR_H_

