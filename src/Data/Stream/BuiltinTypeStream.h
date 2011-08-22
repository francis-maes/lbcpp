/*-----------------------------------------.---------------------------------.
| Filename: BuiltinTypeStream.h            | BuiltinType Streams             |
| Author  : Julien Becker                  |                                 |
| Started : 09/07/2011 11:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_DATA_STREAM_BUILTINTYPE_H_
# define LBCPP_DATA_STREAM_BUILTINTYPE_H_

# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

template <class ElementsType>
class BuiltinTypeStream : public Stream
{
public:
  BuiltinTypeStream(TypePtr elementsType, const std::vector<ElementsType>& values)
    : elementsType(elementsType), values(values), currentPosition(0) {}
  BuiltinTypeStream() : currentPosition(0) {}
  
  virtual TypePtr getElementsType() const
    {return elementsType;}
  
  virtual ProgressionStatePtr getCurrentPosition() const
    {return new ProgressionState(currentPosition, values.size(), T("Value"));}
  
  virtual Variable next()
  {
    jassert(currentPosition < values.size());
    return Variable(values[currentPosition++], elementsType);
  }

  virtual bool rewind()
  {
    currentPosition = 0;
    return true;
  }

  virtual bool isExhausted() const
    {return currentPosition == values.size();}

protected:
  friend class DoubleStreamClass;
  friend class IntegerStreamClass;
  friend class BooleanStreamClass;

  TypePtr elementsType;

  std::vector<ElementsType> values;
  size_t currentPosition;
};

extern ClassPtr doubleStreamClass(TypePtr elementsType);

class DoubleStream : public BuiltinTypeStream<double>
{
public:
  DoubleStream(TypePtr elementsType, const std::vector<double>& values)
    : BuiltinTypeStream<double>(elementsType, values)
    {setThisClass(doubleStreamClass(elementsType));}
  DoubleStream() {}
};

extern ClassPtr integerStreamClass(TypePtr elementsType);

class IntegerStream : public BuiltinTypeStream<int>
{
public:
  IntegerStream(TypePtr elementsType, const std::vector<int>& values)
    : BuiltinTypeStream<int>(elementsType, values)
    {setThisClass(integerStreamClass(elementsType));}
  IntegerStream() {}
};

extern ClassPtr booleanStreamClass(TypePtr elementsType);

class BooleanStream : public BuiltinTypeStream<bool>
{
public:
  BooleanStream(const std::vector<bool>& values)
    : BuiltinTypeStream<bool>(booleanType, values)
    {setThisClass(booleanStreamClass(booleanType));}
  BooleanStream(bool value = true)
    : BuiltinTypeStream<bool>(booleanType, std::vector<bool>(1, value))
    {setThisClass(booleanStreamClass(booleanType));}
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_STREAM_BUILTINTYPE_H_
