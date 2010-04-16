/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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

/*!
** @class DenseVector
** @brief Dense vector are vectors where every features are specify whatever the values are.
*/

class DenseVector : public FeatureGeneratorDefaultImplementations<DenseVector, FeatureVector>
{
public:
  typedef FeatureGeneratorDefaultImplementations<DenseVector, FeatureVector> BaseClass;

  /**
  ** Copy constructor.
  **
  ** @param otherVector : original dense vector.
  **
  ** @return a DenseVector instance.
  */
  DenseVector(const DenseVector& otherVector);

  /**
  ** Constructor.
  **
  ** @param dictionary : feature dictionary.
  ** @param values : features values.
  **
  ** @return a DenseVector instance.
  */
  DenseVector(FeatureDictionaryPtr dictionary, const std::vector<double>& values);

  /**
  ** Constructor.
  **
  ** @param dictionary : feature dictionary.
  ** @param initialNumValues : initial number of values.
  ** @param initialNumSubVectors : initial number of subvectors.
  **
  ** @return a DenseVector instance.
  */
  DenseVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr(), size_t initialNumValues = 0, size_t initialNumSubVectors = 0);

  /**
  ** Destructor.
  */
  virtual ~DenseVector()
    {clear();}

  /**
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
  /**
  ** Checks if some features are setted.
  **
  ** @return True if any feature is setted.
  */
  bool hasValues() const
    {return values.size() > 0;}

  /**
  ** Returns a vector containing all DenseVector values.
  **
  ** @return a vector of values.
  */
  const std::vector<double>& getValues() const
    {return values;}

  /**
  ** Returns the number of values.
  **
  ** @return the number of values.
  */
  size_t getNumValues() const
    {return values.size();}

  /**
  ** Feature value setter.
  **
  ** @param index : feature index.
  ** @param value : feature value.
  */
  void set(size_t index, double value)
    {ensureSize(values, index + 1, 0.0); values[index] = value;}

  void set(const String& name, double value)
    {jassert(dictionary); set(dictionary->addFeature(name), value);}

  /**
  ** Feature value getter.
  **
  ** @param index : feature index.
  **
  ** @return feature value at the index @a index.
  */
  double get(size_t index) const
    {return index < values.size() ? values[index] : 0.0;}

  /**
  ** Returns the value at the index @a index.
  **
  ** @param index : feature index.
  **
  ** @return the value at the index @a index.
  */
  double& get(size_t index)
    {ensureSize(values, index + 1, 0.0); return values[index];}

  /**
  ** Returns the index of the maximum value.
  **
  ** @return the index of the maximum value.
  */
  int findIndexOfMaximumValue() const;

  /**
  ** Returns the maximum value.
  **
  ** @return the maximum value.
  */
  double findMaximumValue() const;

  /**
  ** Computes the logarithmic sum of exponentials.
  **
  ** @return log(sum_i exp(x_i)), avoiding numerical errors with too
  ** big exponentials.
  */
  double computeLogSumOfExponentials() const;

  /*
  ** Sub Vectors
  */
  /**
  ** Checks if any subvector is setted.
  **
  ** @return True if any subvector is setted.
  */
  bool hasSubVectors() const
    {return subVectors.size() > 0;}

  /**
  ** Returns the number of subvectors.
  **
  ** @return the number of subvectors.
  */
  size_t getNumSubVectors() const
    {return subVectors.size();}

  /**
  ** Subvector geter.
  **
  ** @param index : subvector index.
  **
  ** @return a DenseVector reference.
  */
  DenseVectorPtr& getSubVector(size_t index)
    {ensureSize(subVectors, index + 1, DenseVectorPtr()); return subVectors[index];}

  DenseVectorPtr getSubVector(size_t index) const
    {return index < subVectors.size() ? subVectors[index] : DenseVectorPtr();}

  /**
  ** Subvector setter.
  **
  ** @param index : subvector index.
  ** @param subVector : subvector value.
  */
  void setSubVector(size_t index, DenseVectorPtr subVector)
  {
    ensureSize(subVectors, index + 1, DenseVectorPtr());
    subVectors[index] = subVector;
  }

  void setSubVector(const String& name, DenseVectorPtr subVector)
    {jassert(dictionary); setSubVector(dictionary->getScopes()->add(name), subVector);}

  /*
  ** Operations
  */
  /**
  ** Size getter (number of values, recursive).
  **
  ** @return the number of values.
  */
  size_t size() const;

  /**
  ** Scalar product.
  **
  ** @param featureGenerator : right operand.
  **
  ** @return scalar product result.
  */
  virtual double dotProduct(const FeatureGeneratorPtr featureGenerator) const;

  /**
  ** Multiplication by a scalar value.
  **
  ** @param scalar : scalar value.
  */
  void multiplyByScalar(double scalar);

  /**
  ** Randomly initializes the dense vector.
  **
  ** @param mean : mean.
  ** @param standardDeviation : standard deviation.
  */
  void initializeRandomly(double mean = 0.0, double standardDeviation = 1.0);

  /**
  ** Adds a weighted dense vector.
  **
  ** @param otherVector : dense vector.
  ** @param weight : scalar value.
  */
  void addWeighted(const DenseVectorPtr otherVector, double weight);

  /**
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

  /**
  ** Adds a dense vector.
  **
  ** @param otherVector : dense vector.
  */
  void add(const DenseVectorPtr otherVector);

  /**
  ** Adds a feature generator (read only dense vector).
  **
  ** @param featureGenerator : feature generator.
  */
  void add(const FeatureGeneratorPtr featureGenerator)
    {featureGenerator->addTo(DenseVectorPtr(this));}

  /**
  ** Substracts a feature generator (read only dense vector).
  **
  ** @param featureGenerator
  */
  void substract(const FeatureGeneratorPtr featureGenerator)
    {featureGenerator->substractFrom(DenseVectorPtr(this));}

  /*
  ** EditableFeatureGenerator
  */
  /**
  ** Clears all (values, subvectors, dictionary).
  **
  */
  virtual void clear()
    {values.clear(); subVectors.clear(); dictionary = FeatureDictionaryPtr();}

  /*
  ** Static FeatureGenerator
  */
  /**
  ** Visitor entry point.
  **
  ** @param visitor : feature visitor.
  ** @see FeatureVisitor
  */
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor) const;

  /*
  ** FeatureGenerator
  */
  /**
  ** Feature dictionary getter.
  **
  ** @return a feature dictionary pointer.
  */
  virtual FeatureDictionaryPtr getDictionary() const;

  /**
  ** Checks if the vector is dense.
  **
  ** @return True if the vector is dense.
  */
  virtual bool isDense() const
    {return true;}

  /**
  ** Converts vector to dense vector.
  **
  ** @return a dense vector pointer.
  */
  virtual DenseVectorPtr toDenseVector() const
    {return DenseVectorPtr(const_cast<DenseVector* >(this));}

  /**
  ** Returns the number of subvectors.
  **
  ** @return the number of subvectors
  */
  virtual size_t getNumSubGenerators() const
    {return subVectors.size();}

  /**
  ** @see FeatureGenerator::getSubGenerator
  */
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const
    {jassert(num < subVectors.size()); return subVectors[num];}

  /**
  ** @see FeatureGenerator::getSubGeneratorIndex
  */
  virtual size_t getSubGeneratorIndex(size_t num) const
    {jassert(num < subVectors.size()); return num;}

  /**
  ** @see FeatureGenerator::getSubGeneratorWithIndex
  */
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const
    {return index < subVectors.size() ? (FeatureGeneratorPtr)subVectors[index] : FeatureGeneratorPtr();}

  /*
  ** Object
  */
  /**
  ** Loads dense vector from a stream.
  **
  ** @param istr : input stream.
  **
  ** @return False if any error occurs.
  */
  virtual bool load(InputStream& istr);

  /**
  ** Saves dense vector to a stream.
  **
  ** @param ostr : output stream.
  */
  virtual void save(OutputStream& ostr) const;

  /**
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
      if (vector.size() && (minimumSize > (100 * vector.size())))
        std::cerr << "Warning: really big feature index " << minimumSize - 1 << " actual size is " << vector.size()
                  << "! Use SparseVectors or change your feature indices." << std::endl;
      vector.resize(minimumSize, defaultValue);
    }
  }

private:
  std::vector<double> values;
  std::vector<DenseVectorPtr> subVectors;

  double denseDotProduct(const DenseVectorPtr otherVector) const;
};

/**
** Loads dense vector from file @a filename.
**
** @param filename : input file name.
**
** @return a new DenseVector instance.
*/
inline DenseVectorPtr loadDenseVector(const File& file)
  {return Object::createFromFileAndCast<DenseVector>(file);}

}; /* namespace lbcpp */

#endif // !LBCPP_DENSE_VECTOR_H_

