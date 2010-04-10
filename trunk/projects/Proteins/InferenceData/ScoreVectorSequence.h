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
#if 0
  /*
  ** InterdependantVariableSet
  */
  virtual VariableType getVariablesType() const
    {return numericVariable;}

  virtual size_t getNumVariables() const
    {return getLength() * numScores;}

  virtual bool getVariable(size_t index, double& result) const
  {
    jassert(index < matrix.size());
    result = matrix[index];
    return true;
  }

  virtual void setVariable(size_t index, double value)
  {
    jassert(index < matrix.size());
    matrix[index] = value;
  }

  virtual featureGenerator computeVariableFeatures(size_t index) const
  {
    jassert(index < matrix.size());
    featureSense("score", matrix[index]);
  }

  /*
  ** ScoreVectorSequence
  */
  virtual size_t getNumScores() const = 0;

  virtual size_t getLength() const
    {return length;}

  virtual void setLength(size_t newLength, bool clearPreviousContent = true)
  {
    if (clearPreviousContent)
    {
      matrix.clear();
      length = newLength;
      numScores = getNumScores();
      if (length && numScores)
        matrix.resize(length * numScores, 0.0);
    }
    else
    {
      // not implemented yet
      jassert(false);
    }
  }

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

  virtual featureGenerator elementFeatures(size_t position) const
  {
    size_t startIndex = position * numScores;
    for (size_t i = 0; i < numScores; ++i)
    {
      double score = matrix[startIndex + i];
      if (score)
        featureSense(i, score);
    }
  }

  double getScore(size_t position, size_t scoreIndex) const
    {return matrix[getIndex(position, scoreIndex)];}

  void setScore(size_t position, size_t scoreIndex, double value)
    {matrix[getIndex(position, scoreIndex)] = value;}
#endif // 0

protected:
  FeatureDictionaryPtr dictionary;
  size_t length, numScores;
  std::vector<double> matrix;

  virtual bool load(InputStream& istr)
  {return lbcpp::read(istr, length) && lbcpp::read(istr, numScores) && lbcpp::read(istr, matrix);}

  virtual void save(OutputStream& ostr) const
  {
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
