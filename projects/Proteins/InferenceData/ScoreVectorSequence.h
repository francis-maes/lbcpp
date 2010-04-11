/*-----------------------------------------.---------------------------------.
| Filename: ScoreVectorSequence.h          | Score Vector Sequence           |
| Author  : Francis Maes                   |                                 |
| Started : 26/03/2010 18:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_DATA_SCORE_VECTOR_SEQUENCE_H_
# define LBCPP_INFERENCE_DATA_SCORE_VECTOR_SEQUENCE_H_

# include "Sequence.h"

namespace lbcpp
{

class ScoreVectorSequence : public Sequence
{
public:
  ScoreVectorSequence(FeatureDictionaryPtr dictionary, size_t length)
    : dictionary(dictionary), length(length), numScores(dictionary->getNumFeatures()), matrix(length * dictionary->getNumFeatures(), 0.0) {}
  ScoreVectorSequence() {}

  /*
  ** ObjectContainer
  */
  virtual size_t size() const
    {return length;}
  
  virtual void resize(size_t newSize)
  {
    length = newSize;
    matrix.clear();
    matrix.resize(length * numScores, 0.0);
  }

  virtual ObjectPtr get(size_t position) const
  {
    jassert(position < length);
    DenseVectorPtr res = new DenseVector(dictionary, numScores);
    size_t startIndex = position * numScores;
    for (size_t i = 0; i < numScores; ++i)
      res->set(i, matrix[startIndex + i]);
    return res;
  }

  virtual void set(size_t position, ObjectPtr object)
  {
    jassert(position < length);
    DenseVectorPtr vector = object.dynamicCast<DenseVector>();
    jassert(vector);
    jassert(vector->getDictionary() == dictionary);
    size_t startIndex = position * numScores;
    for (size_t i = 0; i < numScores; ++i)
      matrix[startIndex + i] = vector->get(i);
  }

  virtual FeatureGeneratorPtr elementFeatures(size_t position) const
    {return get(position).dynamicCast<FeatureGenerator>();}

  virtual String elementToString(size_t position) const
  {
    String res;
    for (size_t i = 0; i < numScores; ++i)
    {
      if (res.isNotEmpty())
        res += T(", ");
      res += String(getScore(position, i));
    }
    return res + T("\n");
  }
  
  double getScore(size_t position, size_t scoreIndex) const
    {jassert(position < length && scoreIndex < numScores); return matrix[getIndex(position, scoreIndex)];}

  void setScore(size_t position, size_t scoreIndex, double value)
    {jassert(position < length && scoreIndex < numScores); matrix[getIndex(position, scoreIndex)] = value;}

  void setScore(size_t position, const String& scoreName, double value)
  {
    int index = dictionary->getFeatures()->getIndex(scoreName);
    jassert(index >= 0);
    setScore(position, (size_t)index, value);
  }

  FeatureDictionaryPtr getDictionary() const
    {return dictionary;}

protected:
  FeatureDictionaryPtr dictionary;
  size_t length, numScores;
  std::vector<double> matrix;

  virtual bool load(InputStream& istr)
  {
    dictionary = FeatureDictionaryManager::getInstance().readDictionaryNameAndGet(istr);
    return dictionary && lbcpp::read(istr, length) && lbcpp::read(istr, numScores) && lbcpp::read(istr, matrix);
  }

  virtual void save(OutputStream& ostr) const
  {
    lbcpp::write(ostr, dictionary->getName());
    lbcpp::write(ostr, length);
    lbcpp::write(ostr, numScores);
    lbcpp::write(ostr, matrix);
  }

  size_t getIndex(size_t position, size_t scoreIndex) const
  {
    jassert(position < length);
    jassert(scoreIndex < numScores);
    return scoreIndex + position * numScores;
  }
};

typedef ReferenceCountedObjectPtr<ScoreVectorSequence> ScoreVectorSequencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_SCORE_VECTOR_SEQUENCE_H_
