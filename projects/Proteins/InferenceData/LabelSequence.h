/*-----------------------------------------.---------------------------------.
| Filename: LabelSequence.h                | Label Sequence                  |
| Author  : Francis Maes                   |                                 |
| Started : 26/03/2010 12:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_DATA_LABEL_SEQUENCE_H_
# define LBCPP_INFERENCE_DATA_LABEL_SEQUENCE_H_

# include "Sequence.h"
# include "AccumulatedScoresMatrix.h"

namespace lbcpp
{

class LabelSequence : public Sequence
{
public:
  LabelSequence(const String& name, FeatureDictionaryPtr dictionary, size_t length)
    : Sequence(name), dictionary(dictionary), sequence(length, 255)
    {jassert(dictionary->getNumFeatures() < 255);}

  LabelSequence() {}

  /*
  ** ObjectContainer
  */
  virtual size_t size() const
    {return sequence.size();}

  virtual void resize(size_t newSize)
    {sequence.resize(newSize, 255); validateModification();}

  virtual void set(size_t position, ObjectPtr object)
  {
    LabelPtr label = object.dynamicCast<lbcpp::Label>();
    jassert(label && label->getDictionary() == dictionary);
    jassert(position < sequence.size());
    sequence[position] = (unsigned char)label->getIndex();
    validateModification();
  }

  virtual ObjectPtr get(size_t index) const
  {
    jassert(index < sequence.size());
    if (sequence[index] == 255)
      return ObjectPtr();
    return new Label(dictionary, (size_t)sequence[index]);
  }

  void setIndex(size_t position, size_t index)
  {
    jassert(position < sequence.size() && index <= 255);
    sequence[position] = (unsigned char)index;
    validateModification();
  }

  void setString(size_t position, const String& string)
  {
    int index = dictionary->getFeatures()->getIndex(string);
    jassert(index >= 0);
    if (index >= 0)
      setIndex(position, (size_t)index);
  }

  void clear(size_t position)
    {setIndex(position, 255); validateModification();}


  /*
  ** Sequence
  */
  virtual FeatureGeneratorPtr elementFeatures(size_t position) const
    {return get(position).dynamicCast<FeatureGenerator>();}

  virtual FeatureGeneratorPtr sumFeatures(size_t begin, size_t end) const
    {const_cast<LabelSequence* >(this)->ensureAccumulatorsAreComputed(); return accumulators->sumFeatures(begin, end);}

  /*
  ** Serialization
  */
  virtual ObjectPtr clone() const
  {
    ReferenceCountedObjectPtr<LabelSequence> res = Object::createAndCast<LabelSequence>(getClassName());
    res->dictionary = dictionary;
    res->sequence = sequence;
    return res;
  }

  FeatureDictionaryPtr getDictionary() const
    {return dictionary;}

protected:
  virtual bool load(InputStream& istr)
  {
    dictionary = FeatureDictionaryManager::getInstance().readDictionaryNameAndGet(istr);
    if (!dictionary || !lbcpp::read(istr, sequence))
      return false;
    validateModification();
    return true;
  }
  
  virtual void save(OutputStream& ostr) const
  {
    lbcpp::write(ostr, dictionary->getName());
    lbcpp::write(ostr, sequence);
  }

protected:
  FeatureDictionaryPtr dictionary;
  
  // Note: most sequence labeling tasks involve less than 255 labels,
  //  so we use a single byte per element in order to spare memory
  // Labels: 0..254
  // Special value for unlabeled elements: 255
  std::vector<unsigned char> sequence;

  AccumulatedScoresMatrixPtr accumulators;

  void validateModification()
    {accumulators = AccumulatedScoresMatrixPtr();}

  void ensureAccumulatorsAreComputed();
};

typedef ReferenceCountedObjectPtr<LabelSequence> LabelSequencePtr;


}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_LABEL_SEQUENCE_H_
