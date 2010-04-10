/*-----------------------------------------.---------------------------------.
| Filename: Sequence.h                     | Sequence base class             |
| Author  : Francis Maes                   |                                 |
| Started : 26/03/2010 18:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_DATA_SEQUENCE_H_
# define LBCPP_INFERENCE_DATA_SEQUENCE_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class Sequence : public ObjectContainer
{
public:
  virtual String elementToString(size_t position) const
  {
    ObjectPtr object = get(position);
    return object ? object->toString() : T("<null>");
  }

  virtual FeatureGeneratorPtr elementFeatures(size_t position) const = 0;

  virtual String toString() const;

  FeatureGeneratorPtr possiblyOutOfBoundsElementFeatures(int position) const;
  FeatureGeneratorPtr windowFeatures(size_t position, size_t numPrevs, size_t numNexts, bool includeCurrent) const;

  FeatureGeneratorPtr frequencyFeatures(int startPosition, int endPosition) const;
  FeatureGeneratorPtr symetricFrequencyFeatures(size_t position, size_t size, bool includeCurrent) const;
};

typedef ReferenceCountedObjectPtr<Sequence> SequencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_SEQUENCE_H_
