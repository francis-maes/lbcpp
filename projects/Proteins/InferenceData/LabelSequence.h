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
  LabelSequence(const String& name, FeatureDictionaryPtr dictionary, size_t length = 0);
  LabelSequence() {}

  FeatureDictionaryPtr getDictionary() const
    {return dictionary;}

  String getString(size_t position) const;
  int getIndex(size_t position) const;

  void setIndex(size_t position, size_t index);
  void setString(size_t position, const String& string);

  void append(size_t index);

  /*
  ** ObjectContainer
  */
  virtual size_t size() const
    {return sequence.size();}

  virtual void resize(size_t newSize);
  virtual void set(size_t position, ObjectPtr object);
  virtual ObjectPtr get(size_t index) const;
  virtual bool hasObject(size_t index) const;
  void clear(size_t position);

  /*
  ** Sequence
  */
  virtual FeatureGeneratorPtr elementFeatures(size_t position) const;
  virtual FeatureGeneratorPtr sumFeatures(size_t begin, size_t end) const;

  String getSegmentConjunctionFeatureName(size_t beginPosition, size_t segmentCount, bool forward = true) const;
  FeatureGeneratorPtr bidirectionalSegmentConjunctionFeatures(size_t position, size_t segmentCount) const;

  /*
  ** Serialization
  */
  virtual ObjectPtr clone() const;

protected:
  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;

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
