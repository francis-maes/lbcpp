/*-----------------------------------------.---------------------------------.
| Filename: ScalarSequence.h               | Scalar Sequence                 |
| Author  : Francis Maes                   |                                 |
| Started : 26/04/2010 11:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_DATA_SCALAR_SEQUENCE_H_
# define LBCPP_INFERENCE_DATA_SCALAR_SEQUENCE_H_

# include "LabelSequence.h"

namespace lbcpp
{

class ScalarSequence : public Sequence
{
public:
  ScalarSequence(const String& name, size_t length = 0);
  ScalarSequence() {}

  /*
  ** ScalarSequence
  */
  bool hasValue(size_t position) const;
  double getValue(size_t position) const;

  void setValue(size_t position, double value);
  void removeValue(size_t position);
  void appendValue(double value);

  LabelSequencePtr makeBinaryLabelSequence(const String& name, double threshold) const;

  /*
  ** ObjectContainer
  */
  virtual size_t size() const
    {return sequence.size();}

  virtual void resize(size_t newSize);
  virtual void set(size_t position, ObjectPtr object);
  virtual ObjectPtr get(size_t index) const;
  virtual bool hasObject(size_t index) const;

  /*
  ** Sequence
  */
  virtual FeatureGeneratorPtr elementFeatures(size_t position) const;
  virtual FeatureGeneratorPtr sumFeatures(size_t begin, size_t end) const;

  /*
  ** Serialization
  */
  virtual ObjectPtr clone() const;

protected:
  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;

protected:
  std::vector<double> sequence;

  AccumulatedScoresMatrixPtr accumulators;

  void validateModification()
    {accumulators = AccumulatedScoresMatrixPtr();}

  void ensureAccumulatorsAreComputed();
};

typedef ReferenceCountedObjectPtr<ScalarSequence> ScalarSequencePtr;


}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_SCALAR_SEQUENCE_H_
