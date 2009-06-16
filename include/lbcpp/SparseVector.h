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
**@brief  #FIXME: all
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
** @brief
*/

class SparseVector : public FeatureGeneratorDefaultImplementations<SparseVector, FeatureVector>
{
public:
  typedef FeatureGeneratorDefaultImplementations<SparseVector, FeatureVector> BaseClass;

  /*!
  **
  **
  ** @param otherVector
  **
  ** @return
  */
  SparseVector(const SparseVector& otherVector);

  /*!
  **
  **
  ** @param reserveNumSubVectors
  **
  ** @return
  */
  SparseVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr(), size_t reserveNumValues = 0, size_t reserveNumSubVectors = 0);

  /*!
  **
  **
  **
  ** @return
  */
  virtual ~SparseVector()
    {clear();}

  /*!
  **
  **
  */
  virtual void clear();

  /*!
  **
  **
  ** @param otherVector
  **
  ** @return
  */
  SparseVector& operator =(const SparseVector& otherVector);

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
  size_t getNumValues() const
    {return values.size();}

  /*!
  **
  **
  ** @param index
  ** @param value
  */
  void set(size_t index, double value);
  /*!
  **
  **
  ** @param name
  ** @param value
  */
  void set(const std::string& name, double value);
  /*!
  **
  **
  ** @param path
  ** @param value
  */
  void set(const std::vector<std::string>& path, double value);

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  double get(size_t index) const;
  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  double& get(size_t index);

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
  SparseVectorPtr getSubVector(size_t index) const;

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  SparseVectorPtr& getSubVector(size_t index);

  /*!
  **
  **
  ** @param index
  ** @param subVector
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
  ** @param scalar
  */
  void multiplyByScalar(double scalar);

  /*!
  **
  **
  ** @param featureGenerator
  ** @param weight
  */
  void addWeighted(const FeatureGeneratorPtr featureGenerator, double weight)
    {featureGenerator->addWeightedTo(SparseVectorPtr(this), weight);}

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
  virtual SparseVectorPtr toSparseVector() const
    {return SparseVectorPtr(const_cast<SparseVector* >(this));}

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
    {assert(num < subVectors.size()); return subVectors[num].second;}

  /*!
  **
  **
  ** @param num
  **
  ** @return
  */
  virtual size_t getSubGeneratorIndex(size_t num) const
    {assert(num < subVectors.size()); return subVectors[num].first;}

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const
    {return getSubVector(index);}

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
    {return new SparseVector(*this);}

private:
  typedef std::vector<std::pair<size_t, double> > FeatureVector;
  typedef std::vector<std::pair<size_t, SparseVectorPtr> > SubVectorVector;

  FeatureVector   values;       /*!< */
  SubVectorVector subVectors;   /*!< */
};

  /*!
  **
  **
  ** @param filename
  **
  ** @return
  */
inline SparseVectorPtr loadSparseVector(const std::string& filename)
  {return Object::loadFromFileAndCast<SparseVector>(filename);}

}; /* namespace lbcpp */

#endif // !LBCPP_SPARSE_VECTOR_H_

