/*-----------------------------------------.---------------------------------.
| Filename: ScoreVectorSequence.cpp        | Score Vector Sequence           |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 16:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ScoreVectorSequence.h"
using namespace lbcpp;

ScoreVectorSequence::ScoreVectorSequence(const String& name, FeatureDictionaryPtr dictionary, size_t length, size_t numScores)
  : Sequence(name), dictionary(dictionary), length(length), numScores(numScores)
{
  if (!numScores)
    this->numScores = dictionary->getNumFeatures();
  matrix.resize(length * this->numScores, 0.0);
}

LabelSequencePtr ScoreVectorSequence::makeArgmaxLabelSequence(const String& name) const
{
  size_t n = size();
  LabelSequencePtr res = new LabelSequence(name, getDictionary(), n);
  for (size_t i = 0; i < n; ++i)
  {
    double bestScore = -DBL_MAX;
    size_t bestScoreIndex = 0;
    for (size_t j = 0; j < numScores; ++j)
    {
      double score = getScore(i, j);
      if (score > bestScore)
        bestScore = score, bestScoreIndex = j;
    }
    res->setIndex(i, bestScoreIndex);
  }
  return res;
}

void ScoreVectorSequence::resize(size_t newSize)
{
  length = newSize;
  matrix.clear();
  matrix.resize(length * numScores, 0.0);
  validateModification();
}

ObjectPtr ScoreVectorSequence::get(size_t position) const
{
  jassert(position < length);
  DenseVectorPtr res = new DenseVector(dictionary, numScores);
  size_t startIndex = position * numScores;
  for (size_t i = 0; i < numScores; ++i)
    res->set(i, matrix[startIndex + i]);
  return res;
}

void ScoreVectorSequence::set(size_t position, ObjectPtr object)
{
  jassert(position < length);
  DenseVectorPtr vector = object.dynamicCast<DenseVector>();
  jassert(vector);
  jassert(vector->getDictionary() == dictionary);
  size_t startIndex = position * numScores;
  for (size_t i = 0; i < numScores; ++i)
    matrix[startIndex + i] = vector->get(i);
  validateModification();
}

double ScoreVectorSequence::getScore(size_t position, size_t scoreIndex) const
{
  jassert(position < length && scoreIndex < numScores);
  return matrix[getIndex(position, scoreIndex)];
}

void ScoreVectorSequence::setScore(size_t position, size_t scoreIndex, double value)
{
  jassert(position < length && scoreIndex < numScores);
  matrix[getIndex(position, scoreIndex)] = value;
  validateModification();
}

void ScoreVectorSequence::setScore(size_t position, const String& scoreName, double value)
{
  int index = dictionary->getFeatures()->getIndex(scoreName);
  jassert(index >= 0);
  setScore(position, (size_t)index, value);
}

FeatureGeneratorPtr ScoreVectorSequence::elementFeatures(size_t position) const
  {return get(position).dynamicCast<FeatureGenerator>();}

FeatureGeneratorPtr ScoreVectorSequence::sumFeatures(size_t begin, size_t end) const
{
  const_cast<ScoreVectorSequence* >(this)->ensureAccumulatorsAreComputed();
  return accumulators->sumFeatures(begin, end);
}

String ScoreVectorSequence::elementToString(size_t position) const
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

ObjectPtr ScoreVectorSequence::clone() const
{
  ScoreVectorSequencePtr res = Object::createAndCast<ScoreVectorSequence>(getClassName());
  res->dictionary = dictionary;
  res->length = length;
  res->numScores = numScores;
  res->matrix = matrix;
  res->name = name;
  return res;
}

bool ScoreVectorSequence::load(InputStream& istr)
{
  if (!Sequence::load(istr))
    return false;

  dictionary = FeatureDictionaryManager::getInstance().readDictionaryNameAndGet(istr);
  if (!dictionary || !lbcpp::read(istr, length) || !lbcpp::read(istr, numScores) || !lbcpp::read(istr, matrix))
    return false;

  validateModification();
  return true;
}

void ScoreVectorSequence::save(OutputStream& ostr) const
{
  Sequence::save(ostr);
  lbcpp::write(ostr, dictionary->getName());
  lbcpp::write(ostr, length);
  lbcpp::write(ostr, numScores);
  lbcpp::write(ostr, matrix);
}

size_t ScoreVectorSequence::getIndex(size_t position, size_t scoreIndex) const
{
  jassert(position < length);
  jassert(scoreIndex < numScores);
  return scoreIndex + position * numScores;
}

void ScoreVectorSequence::ensureAccumulatorsAreComputed()
{
  if (!accumulators)
  {
    accumulators = new AccumulatedScoresMatrix(dictionary, size());
    for (size_t i = 0; i < size(); ++i)
    {
      std::vector<double>& accumulatedScores = accumulators->getAccumulatedScores(i);
      if (i > 0)
        accumulatedScores = accumulators->getAccumulatedScores(i - 1);
      if (accumulatedScores.size() < numScores + 1)
        accumulatedScores.resize(numScores + 1, 0.0);
      for (size_t scoreIndex = 0; scoreIndex < numScores; ++scoreIndex)
        accumulatedScores[scoreIndex + 1] += getScore(i, scoreIndex);
    }
  }
}
