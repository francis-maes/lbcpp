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
  LabelPtr label = object.dynamicCast<lbcpp::Label>();
  jassert(label && label->getDictionary() == dictionary);
  jassert(position < sequence.size());
  sequence[position] = (unsigned char)label->getIndex();
  validateModification();
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
