/*-----------------------------------------.---------------------------------.
| Filename: MatlabFileParser.h             | Matlab File parser              |
| Author  : Julien Becker                  |                                 |
| Started : 08/11/2010 21:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MNIST_MATLAB_FILE_PARSER_H_
# define LBCPP_MNIST_MATLAB_FILE_PARSER_H_

# include <lbcpp/lbcpp.h>
# include "Image/Image.h"

namespace lbcpp
{

extern EnumerationPtr digitTypeEnumeration;
extern ClassPtr mnistImageClass;  

class MNISTImage : public Object
{
public:
  enum {numPixels = 784};
  
  MNISTImage(const std::vector<double>& pixels, size_t digit)
  : pixels(pixels), digit(digit)
  {}
  
  MNISTImage() {}
  
  void setPixels(const std::vector<double>& pixels)
  {this->pixels = pixels;}
  
  const std::vector<double>& getPixels() const
  {return pixels;}
  
  Variable getDigit() const
  {return Variable(digit, digitTypeEnumeration);}
protected:
  friend class MNISTImageClass;
  
  std::vector<double> pixels;
  size_t digit;
};

typedef ReferenceCountedObjectPtr<MNISTImage> MNISTImagePtr;
  
class MatlabFileParser : public TextParser
{
public:
  MatlabFileParser(ExecutionContext& context, const File& file)
    : TextParser(context, file)
  {}
  
  virtual TypePtr getElementsType() const
    {return mnistImageClass;}
  
  virtual void parseBegin(ExecutionContext& context) {}

  virtual bool parseLine(ExecutionContext& context, const String& line)
  {
    if (line.trim() == String::empty)
      return true;
    
    std::vector<String> values;
    tokenize(line, values);

    //jassert(MNISTImage::numPixels <= values.size() <= MNISTImage::numPixels + 1);
    std::vector<double> pixels(MNISTImage::numPixels);
    for (size_t i = 0; i < MNISTImage::numPixels; ++i)
      pixels[i] = values[i].getDoubleValue();

    size_t missingValue = Variable::missingValue(digitTypeEnumeration).getInteger();
    size_t digit = missingValue;
    if (values.size() == MNISTImage::numPixels + 1) 
    {
      //jassert(0 <= values[MNISTImage::numPixels].getIntValue() < missingValue);
      digit = values[MNISTImage::numPixels].getIntValue();
    }

    setResult(Variable(MNISTImagePtr(new MNISTImage(pixels, digit)), mnistImageClass));
    return true;
  }
  
  virtual bool parseEnd(ExecutionContext& context)
    {return true;}
};
  
extern ContainerPtr parseDataFile(ExecutionContext& context, const File& file);

}; /* namespace lbcpp */

#endif // !LBCPP_MNIST_MATLAB_FILE_PARSER_H_
