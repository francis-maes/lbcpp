#ifndef LBCPP_MNISP_IMAGE_PROGRAM_H_
# define LBCPP_MNISP_IMAGE_PROGRAM_H_

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
  
};

#endif //!LBCPP_MNISP_IMAGE_PROGRAM_H_
