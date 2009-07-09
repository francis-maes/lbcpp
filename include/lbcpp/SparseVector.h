/*-----------------------------------------.---------------------------------.
| Filename: SparseVector.h                 | Sparse composite vectors        |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 18:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   SparseVector.h
**@author Francis MAES
**@date   Mon Jun 15 23:41:50 2009
**
**@brief  Sparse composite vector declarations.
**
**
*/

#ifndef LBCPP_SPARSE_VECTOR_H_
# define LBCPP_SPARSE_VECTOR_H_

# include "EditableFeatureGenerator.h"

namespace lbcpp
{

/*!
** @class SparseVector
** @brief A sparse vector is a vector where only few features are
** specified. Each item is a pair < feature id, value >.
*/
class SparseVector : public FeatureGeneratorDefaultImplementations<SparseVector, FeatureVector>
{
public:
  typedef FeatureGeneratorDefaultImplementations<SparseVector, FeatureVector> BaseClass;

  /**
  ** Copy constructor.
  **
  ** @param otherVector : original sparse vector.
  **
  ** @return a SparseVector instance.
  */
  SparseVector(const SparseVector& otherVector);

  /**
  ** Constructor.
  **
  ** @param dictionary : feature dictionary.
  ** @param reserveNumValues : amount of space to reserve for values.
  ** @param reserveNumSubVectors : amount of space to reserve for subvectors.
  **
  ** @return a SparseVector instance.
  */
  SparseVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr(), size_t reserveNumValues = 0, size_t reserveNumSubVectors = 0);

  /**
  ** Destructor.
  */
  virtual ~SparseVector()
    {clear();}

  /**
  ** Clears vector.
  **
  */
  virtual void clear();

  /**
  ** = operator.
  **
  ** @param otherVector : right operand (SparseVector).
  **
  ** @return a SparseVector reference.
  */
  SparseVector& operator =(const SparseVector& otherVector);

  /*
  ** Features
  */
  /**
  ** Checks if the sparse vector has some values.
  **
  ** @return False if there is no value.
  */
  bool hasValues() const
    {return values.size() > 0;}

  /**
  ** Returns the number of values.
  **
  ** @return the number of values.
  */
  size_t getNumValues() const
    {return values.size();}

  /**
  ** Value setter.
  **
  ** @param index : value index.
  ** @param value : value.
  */
  void set(size_t index, double value);

  /**
  ** Name setter.
  **
  ** @param name : name.
  ** @param value : value.
  */
  void set(const std::string& name, double value);

  /**
  ** Path setter.
  **
  ** @param path : path.
  ** @param value : value.
  */
  void set(const std::vector<std::string>& path, double value);

  /**
  ** Returns the value at the index @a index.
  **
  ** @param index : value index.
  **
  ** @return the value stores at the index @a index.
  */
  double get(size_t index) const;

  /**
  ** Value getter (reference).
  **
  ** @param index : value index.
  **
  ** @return value (reference).
  */
  double& get(size_t index);

  /*
  ** Sub Vectors
  */
  /**
  ** Checks if the sparse vector has subvectors.
  **
  ** @return False if there is no subvectors.
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
  ** Subvector getter.
  **
  ** @param index : subvector index.
  **
  ** @return a subvector pointer.
  */
  SparseVectorPtr getSubVector(size_t index) const;

  /**
  ** Subvector getter (reference).
  **
  ** @param index : subvector index.
  **
  ** @return a subvector reference.
  */
  SparseVectorPtr& getSubVector(size_t index);

  /**
  ** Subvector setter.
  **
  ** @param index : subvector index.
  ** @param subVector : subvector value.
  */
  void setSubVector(size_t index, SparseVectorPtr subVector)
  {
    getSubVector(index) = subVector;
    if (dictionary)
      dictionary->ensureSubDictionary(index, subVector->dictionary);
  }

  /*
  ** Operations
  */
  /**
  ** Returns the number of values (including values of subvectors).
  **
  ** @return number of values (including values of subvectors).
  */
  size_t size() const;

  /**
  ** Scalar multiplication.
  **
  ** @param scalar : scalar value.
  */
  void multiplyByScalar(double scalar);

  /**
  ** Adds a weighted feature generator.
  **
  ** @param featureGenerator : feature generator.
  ** @param weight : scalar value.
  */
  void addWeighted(const FeatureGeneratorPtr featureGenerator, double weight)
    {featureGenerator->addWeightedTo(SparseVectorPtr(this), weight);}

  /*
  ** Static FeatureGenerator
  */
  /**
  ** Visitor entry point.
  **
  ** @param visitor : visitor.
  ** @see FeatureVisitor
  */
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor) const;

  /*
  ** FeatureGenerator
  */
  /**
  ** Returns the vector FeatureDictionary.
  **
  ** @return a feature dictionary pointer.
  ** @see FeatureDictionary
  */
  virtual FeatureDictionaryPtr getDictionary() const;

  /**
  ** Converts to a SparseVector.
  **
  ** @return a sparse vector pointer.
  */
  virtual SparseVectorPtr toSparseVector() const
    {return SparseVectorPtr(const_cast<SparseVector* >(this));}

  /**
  ** Returns the number of sub generators.
  **
  ** @return the number of subvectors.
  */
  virtual size_t getNumSubGenerators() const
    {return subVectors.size();}

  /**
  ** Subgenerator getter.
  **
  ** @param num : subvector index.
  **
  ** @return subgenerator.
  */
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const
    {assert(num < subVectors.size()); return subVectors[num].second;}

  /**
  ** Subgenerator index getter.
  **
  ** @param num : subvector index.
  **
  ** @return subgenerator index.
  */
  virtual size_t getSubGeneratorIndex(size_t num) const
    {assert(num < subVectors.size()); return subVectors[num].first;}

  /**
  ** Returns the subgenerator at the index @a index.
  **
  ** @param index : subgenerator index.
  **
  ** @return a new Featuregenerator.
  */
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const
    {return getSubVector(index);}

  /*
  ** Object
  */
  /**
  ** Loads sparse vector from a stream.
  **
  ** @param istr : input stream.
  **
  ** @return False if any error occurs.
  */
  virtual bool load(std::istream& istr);

  /**
  ** Saves sparse vector to a stream.
  **
  ** @param ostr : output stream.
  */
  virtual void save(std::ostream& ostr) const;

  /**
  ** Clones sparse vector.
  **
  ** @return a copy of the current sparse vector.
  */
  virtual ObjectPtr clone() const
    {return new SparseVector(*this);}

private:
  typedef std::vector<std::pair<size_t, double> > FeatureVector;
  typedef std::vector<std::pair<size_t, SparseVectorPtr> > SubVectorVector;

  FeatureVector   values;       /**< Sparse vector values. */
  SubVectorVector subVectors;   /**< Sparse vector subvectors. */
};

  /**
  ** Loads sparse vector from a file.
  **
  ** @param filename : file name.
  **
  ** @return a sparse vector pointer.
  */
inline SparseVectorPtr loadSparseVector(const std::string& filename)
  {return Object::loadFromFileAndCast<SparseVector>(filename);}

}; /* namespace lbcpp */

#endif // !LBCPP_SPARSE_VECTOR_H_

