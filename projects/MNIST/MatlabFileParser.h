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
# include "MNISTImage.h"

namespace lbcpp
{

class MatlabFileParser : public TextParser
{
public:
  MatlabFileParser(const File& file, MessageCallback& callback = MessageCallback::getInstance())
    : TextParser(file, callback)
  {}
  
  virtual TypePtr getElementsType() const
    {return mnistImageClass;}
  
  virtual void parseBegin() {}

  virtual bool parseLine(const String& line)
  {
    if (line.trim() == String::empty)
      return true;
    
    std::vector<String> values;
    tokenize(line, values);

    jassert(MNISTImage::numPixels <= values.size() <= MNISTImage::numPixels + 1);
    std::vector<double> pixels(MNISTImage::numPixels);
    for (size_t i = 0; i < MNISTImage::numPixels; ++i)
      pixels[i] = values[i].getDoubleValue();

    size_t missingValue = Variable::missingValue(digitTypeEnumeration).getInteger();
    size_t digit = missingValue;
    if (values.size() == MNISTImage::numPixels + 1) 
    {
      jassert(0 <= values[MNISTImage::numPixels].getIntValue() < missingValue);
      digit = values[MNISTImage::numPixels].getIntValue();
    }

    setResult(Variable(MNISTImagePtr(new MNISTImage(pixels, digit)), mnistImageClass));
    return true;
  }
  
  virtual bool parseEnd()
    {return true;}
};
  
extern ContainerPtr parseDataFile(const File& file);

}; /* namespace lbcpp */

#endif // !LBCPP_MNIST_MATLAB_FILE_PARSER_H_
