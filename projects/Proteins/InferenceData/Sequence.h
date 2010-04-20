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
  Sequence(const String& name) : ObjectContainer(name) {}
  Sequence() {}

  virtual String elementToString(size_t position) const
  {
    ObjectPtr object = get(position);
    return object ? object->toString() : T("<null>");
  }

  virtual FeatureGeneratorPtr elementFeatures(size_t position) const = 0;
  virtual FeatureGeneratorPtr sumFeatures(size_t begin, size_t end) const = 0; // begin and end are in [0, size()]

  virtual String toString() const;

  FeatureGeneratorPtr possiblyOutOfBoundsElementFeatures(int position) const;
  FeatureGeneratorPtr windowFeatures(size_t position, size_t numPrevs, size_t numNexts, bool includeCurrent) const;

  FeatureGeneratorPtr possiblyOutOfBoundsFrequencyFeatures(int begin, int end) const;
  FeatureGeneratorPtr symetricFrequencyFeatures(size_t position, size_t size, bool includeCurrent) const;
  FeatureGeneratorPtr multiScaleSymetricFrequencyFeatures(size_t position, size_t smallestSize, size_t sizeFactor, size_t numScales, bool includeCurrent) const;
};

typedef ReferenceCountedObjectPtr<Sequence> SequencePtr;

template<class ElementType>
class BuiltinVectorBasedSequence : public Sequence
{
public:
  BuiltinVectorBasedSequence(const String& name, size_t length = 0)
    : Sequence(name), elements(length) {}
  BuiltinVectorBasedSequence() {}

  typedef std::vector<ElementType> VectorType;

  virtual size_t size() const
    {return elements.size();}

  virtual void resize(size_t newSize)
    {elements.resize(newSize);}

   virtual ObjectPtr get(size_t index) const
    {return elementFeatures(index);}

  virtual FeatureGeneratorPtr elementFeatures(size_t position) const
    {return unitFeatureGenerator();}

  virtual FeatureGeneratorPtr sumFeatures(size_t begin, size_t end) const
    {return unitFeatureGenerator();}

protected:
  VectorType elements;

  virtual bool load(InputStream& istr)
    {return Sequence::load(istr) && lbcpp::read(istr, elements);}

  virtual void save(OutputStream& ostr) const
    {Sequence::save(ostr); lbcpp::write(ostr, elements);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_SEQUENCE_H_
