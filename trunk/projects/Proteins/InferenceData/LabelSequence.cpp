/*-----------------------------------------.---------------------------------.
| Filename: LabelSequence.cpp              | Label Sequence                  |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "LabelSequence.h"
using namespace lbcpp;

LabelSequence::LabelSequence(const String& name, FeatureDictionaryPtr dictionary, size_t length)
  : Sequence(name), dictionary(dictionary), sequence(length, 255)
{
  jassert(dictionary && dictionary->getNumFeatures() < 255);
}

String LabelSequence::elementToString(size_t position) const
{
  int index = getIndex(position);
  if (dictionary == BinaryClassificationDictionary::getInstance())
    return (index < 0 ? T("?") : (index == 0 ? T("-") : T("+")));
  else
    return (index < 0 ? T("?") : dictionary->getFeature((size_t)index));
}

void LabelSequence::resize(size_t newSize)
{
  sequence.resize(newSize, 255);
  validateModification();
}

void LabelSequence::set(size_t position, ObjectPtr object)
{
  size_t index;
  LabelPtr label = object.dynamicCast<lbcpp::Label>();
  if (label)
  {
    jassert(label->getDictionary() == dictionary);
    index = (unsigned char)label->getIndex();
  }
  else
  {
    DenseVectorPtr scores = object.dynamicCast<DenseVector>();
    if (scores)
    {
      jassert(scores->getDictionary() == dictionary);
      int i = scores->findIndexOfMaximumValue();
      if (i < 0)
      {
        jassert(false);
        return;
      }
      index = (size_t)i;
    }
    else
    {
      jassert(false);
      return;
    }
  }
  jassert(position < sequence.size());    
  sequence[position] = index;
  validateModification();
}

bool LabelSequence::hasObject(size_t index) const
{
  jassert(index < sequence.size());
  return sequence[index] < 255;
}

ObjectPtr LabelSequence::get(size_t index) const
{
  jassert(index < sequence.size());
  if (sequence[index] == 255)
    return ObjectPtr();
  return new Label(dictionary, (size_t)sequence[index]);
}

String LabelSequence::getString(size_t position) const
{
  jassert(position < sequence.size());
  return sequence[position] == 255 ? T("?") : dictionary->getFeature(sequence[position]);
}

int LabelSequence::getIndex(size_t position) const
{
  jassert(position < sequence.size());
  return sequence[position] == 255 ? -1 : (int)sequence[position];
}

void LabelSequence::setIndex(size_t position, size_t index)
{
  jassert(index < 255);
  jassert(position < sequence.size() && index <= 255);
  sequence[position] = (unsigned char)index;
  validateModification();
}

void LabelSequence::setString(size_t position, const String& string)
{
  int index = dictionary->getFeatures()->getIndex(string);
  jassert(index >= 0);
  if (index >= 0)
    setIndex(position, (size_t)index);
}

void LabelSequence::append(size_t index)
{
  jassert(index < 255);
  sequence.push_back(index);
  validateModification();
}

void LabelSequence::clear(size_t position)
{
  setIndex(position, 255);
  validateModification();
}

FeatureGeneratorPtr LabelSequence::elementFeatures(size_t position) const
  {return get(position).dynamicCast<FeatureGenerator>();}

FeatureGeneratorPtr LabelSequence::sumFeatures(size_t begin, size_t end) const
{
  const_cast<LabelSequence* >(this)->ensureAccumulatorsAreComputed();
  return accumulators->sumFeatures(begin, end);
}

FeatureGeneratorPtr LabelSequence::entropyFeatures(size_t begin, size_t end) const
{
  const_cast<LabelSequence* >(this)->ensureAccumulatorsAreComputed();
  return accumulators->entropyFeatures(begin, end);
}

ObjectPtr LabelSequence::clone() const
{
  LabelSequencePtr res = Object::createAndCast<LabelSequence>(getClassName());
  res->dictionary = dictionary;
  res->sequence = sequence;
  res->name = name;
  return res;
}

bool LabelSequence::load(InputStream& istr)
{
  if (!Sequence::load(istr))
    return false;

  dictionary = FeatureDictionaryManager::getInstance().readDictionaryNameAndGet(istr);
  if (!dictionary || !lbcpp::read(istr, sequence))
    return false;

  validateModification();
  return true;
}

void LabelSequence::save(OutputStream& ostr) const
{
  Sequence::save(ostr);
  lbcpp::write(ostr, dictionary->getName());
  lbcpp::write(ostr, sequence);
}

void LabelSequence::ensureAccumulatorsAreComputed()
{
  if (!accumulators)
  {
    accumulators = new AccumulatedScoresMatrix(dictionary, size());
    for (size_t i = 0; i < size(); ++i)
    {
      std::vector<double>& scores = accumulators->getAccumulatedScores(i);
      if (i > 0)
        scores = accumulators->getAccumulatedScores(i - 1);
      size_t accumulatorNumber = (size_t)(sequence[i] == 255 ? 0 : sequence[i] + 1);
      if (scores.size() <= accumulatorNumber)
        scores.resize(accumulatorNumber + 1, 0.0);
      scores[accumulatorNumber] += 1.0;
    }
  }
}
