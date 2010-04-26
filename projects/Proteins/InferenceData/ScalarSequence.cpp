/*-----------------------------------------.---------------------------------.
| Filename: ScalarSequence.cpp             | Scalar Sequence                 |
| Author  : Francis Maes                   |                                 |
| Started : 26/04/2010 11:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ScalarSequence.h"
using namespace lbcpp;

ScalarSequence::ScalarSequence(const String& name, size_t length)
  : Sequence(name), sequence(length, DBL_MAX) {}

LabelSequencePtr ScalarSequence::makeBinaryLabelSequence(const String& name, double threshold) const
{
  size_t n = size();
  LabelSequencePtr res = new LabelSequence(name, BinaryClassificationDictionary::getInstance(), n);
  for (size_t i = 0; i < n; ++i)
    if (hasValue(i))
      res->setIndex(i, getValue(i) >= threshold ? 1 : 0);
  return res;  
}

void ScalarSequence::resize(size_t newSize)
{
  sequence.resize(newSize, DBL_MAX);
  validateModification();
}

void ScalarSequence::set(size_t position, ObjectPtr object)
{
  ScalarPtr scalar = object.dynamicCast<Scalar>();
  jassert(scalar);
  jassert(position < sequence.size());
  sequence[position] = scalar->getValue();
  validateModification();
}

bool ScalarSequence::hasObject(size_t index) const
{
  jassert(index < sequence.size());
  return sequence[index] != DBL_MAX;
}

ObjectPtr ScalarSequence::get(size_t index) const
{
  jassert(index < sequence.size());
  if (sequence[index] == DBL_MAX)
    return ObjectPtr();
  return new Scalar(sequence[index]);
}

void ScalarSequence::setValue(size_t position, double value)
  {jassert(position < sequence.size()); sequence[position] = value;}

double ScalarSequence::getValue(size_t position) const
  {jassert(position < sequence.size()); return sequence[position];}

bool ScalarSequence::hasValue(size_t position) const
  {jassert(position < sequence.size()); return sequence[position] != DBL_MAX;}

void ScalarSequence::appendValue(double value)
  {sequence.push_back(value); validateModification();}

void ScalarSequence::removeValue(size_t position)
  {setValue(position, DBL_MAX);}

FeatureGeneratorPtr ScalarSequence::elementFeatures(size_t position) const
  {return get(position).dynamicCast<FeatureGenerator>();}

FeatureGeneratorPtr ScalarSequence::sumFeatures(size_t begin, size_t end) const
{
  const_cast<ScalarSequence* >(this)->ensureAccumulatorsAreComputed();
  return accumulators->sumFeatures(begin, end);
}

ObjectPtr ScalarSequence::clone() const
{
  ScalarSequencePtr res = Object::createAndCast<ScalarSequence>(getClassName());
  res->sequence = sequence;
  res->name = name;
  return res;
}

bool ScalarSequence::load(InputStream& istr)
{
  if (!Sequence::load(istr) || !lbcpp::read(istr, sequence))
    return false;
  validateModification();
  return true;
}

void ScalarSequence::save(OutputStream& ostr) const
{
  Sequence::save(ostr);
  lbcpp::write(ostr, sequence);
}

void ScalarSequence::ensureAccumulatorsAreComputed()
{
  if (!accumulators)
  {
    accumulators = new AccumulatedScoresMatrix(unitFeatureGenerator()->getDictionary(), size());
    for (size_t i = 0; i < size(); ++i)
    {
      std::vector<double>& accumulatedScores = accumulators->getAccumulatedScores(i);
      if (i == 0)
        accumulatedScores.resize(1, 0.0);
      else
        accumulatedScores = accumulators->getAccumulatedScores(i - 1);
      accumulatedScores[0] += getValue(i);
    }
  }
}
