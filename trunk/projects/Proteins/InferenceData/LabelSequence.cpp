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
  {jassert(dictionary && dictionary->getNumFeatures() < 255);}


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

static int getSegmentCode(const LabelSequence& sequence, int position)
{
  if (position < 0)
    return -3;
  else if (position >= (int)sequence.size())
    return -2;
  else
    return sequence.hasObject(position) ? (int)sequence.getIndex(position) : -1;
}

static String segmentFeatureName(FeatureDictionaryPtr dictionary, int segmentCode, int length)
{
  if (segmentCode == -3)
    return T(">>");
  if (segmentCode == -2)
    return T("<<");
  if (segmentCode == -1)
    return T("_");

  String res = dictionary->getFeature((size_t)segmentCode);
  if (length < 5)
    res += String(length);
  else if (length < 10)
    res += T("[5-10[");
  else if (length < 20)
    res += T("[10-20[");
  else if (length < 50)
    res += T("[20-50[");
  else if (length < 100)
    res += T("[50-100[");
  else if (length < 200)
    res += T("[100-200[");
  else if (length < 500)
    res += T("[200-500[");
  else
    res += T("[500-oo[");
  return res;
}

String LabelSequence::getSegmentConjunctionFeatureName(size_t beginPosition, size_t segmentCount, bool forward) const
{
  int delta = forward ? 1 : -1;
  int n = (int)this->size();
  jassert(beginPosition < this->size());
  int pos = (int)beginPosition;
  int currentCode = getSegmentCode(*this, pos);
  int currentCodeFirstPos = pos;

  String featureName;
  for (size_t i = 0; i < segmentCount; ++i)
  {
    while (pos >= 0 && pos < n && getSegmentCode(*this, pos) == currentCode)
      pos += delta;
    
    featureName += segmentFeatureName(this->getDictionary(), currentCode, abs(pos - currentCodeFirstPos));
    if (currentCode <= -2)
      break;

    currentCode = getSegmentCode(*this, pos);
    currentCodeFirstPos = pos;
  }
  return featureName;
}
