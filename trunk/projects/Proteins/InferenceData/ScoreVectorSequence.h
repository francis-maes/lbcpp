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
# include "AccumulatedScoresMatrix.h"

namespace lbcpp
{

class ScoreVectorSequence : public Sequence
{
public:
  ScoreVectorSequence(const String& name, FeatureDictionaryPtr dictionary, size_t length);
  ScoreVectorSequence() {}

  FeatureDictionaryPtr getDictionary() const
    {return dictionary;}

  size_t getNumScores() const
    {return numScores;}

  double getScore(size_t position, size_t scoreIndex) const;
  void setScore(size_t position, size_t scoreIndex, double value);
  void setScore(size_t position, const String& scoreName, double value);

  /*
  ** ObjectContainer (contains DenseVectors)
  */
  virtual size_t size() const
    {return length;}
  
  virtual void resize(size_t newSize);
  virtual ObjectPtr get(size_t position) const;
  virtual void set(size_t position, ObjectPtr object);

  /*
  ** Sequence
  */
  virtual FeatureGeneratorPtr elementFeatures(size_t position) const;
  virtual FeatureGeneratorPtr sumFeatures(size_t begin, size_t end) const;
  virtual String elementToString(size_t position) const;

  /*
  ** Object
  */
  virtual ObjectPtr clone() const;

protected:
  FeatureDictionaryPtr dictionary;
  size_t length, numScores;
  std::vector<double> matrix;

  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;

  size_t getIndex(size_t position, size_t scoreIndex) const;

  AccumulatedScoresMatrixPtr accumulators;

  void validateModification()
    {accumulators = AccumulatedScoresMatrixPtr();}

  void ensureAccumulatorsAreComputed();
};

typedef ReferenceCountedObjectPtr<ScoreVectorSequence> ScoreVectorSequencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_SCORE_VECTOR_SEQUENCE_H_
