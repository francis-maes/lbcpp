#ifndef LBCPP_IMAGE_H_
# define LBCPP_IMAGE_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class Image : public Object
{
public:
  Image(size_t width, size_t height) : width(width), height(height), pixels(std::vector<double>(width * height)) {}

  Image() {}
  
  size_t getWidth() const
    {return width;}
  
  size_t getHeight() const
    {return height;}
  
  double getValue(size_t x, size_t y) const
    {return pixels[getIndex(x, y)];}
  
  void setValue(size_t x, size_t y, double value)
    {pixels[getIndex(x, y)] = value;}
  
  virtual TypePtr getImageType() const
    {return probabilityType;}
  
protected:
  friend class ImageClass;
  
  size_t width;
  size_t height;
  std::vector<double> pixels;
  
  size_t getIndex(size_t x, size_t y) const
    {jassert(x < width && y < height); return y * height + x;}
};

typedef ReferenceCountedObjectPtr<Image> ImagePtr;

class BinaryImage : public Image
{
public:
  BinaryImage(size_t width, size_t height) : Image(width, height) {}
  
  BinaryImage() {}

  virtual TypePtr getImageType() const
    {return booleanType;}
  
protected:
  friend class BinaryImageClass;
};

class ImageFunction;
typedef ReferenceCountedObjectPtr<ImageFunction> ImageFunctionPtr;

extern ClassPtr imageClass;
extern ClassPtr binaryImageClass;
  
extern ImageFunctionPtr identityImageFunction(size_t inputWidth, size_t inputHeight);
extern ImageFunctionPtr averageImageFunction(size_t inputWidth, size_t inputHeight, size_t radius);
extern ImageFunctionPtr reduceImageFunction(size_t inputWidth, size_t inputHeight, size_t scaleFactor);
extern ImageFunctionPtr binarizeImageFunction(size_t inputWidth, size_t inputHeight, double threshold);
extern ImageFunctionPtr minimumImageFunction(size_t inputWidth, size_t inputHeight, size_t blockSize);
extern ImageFunctionPtr maximumImageFunction(size_t inputWidth, size_t inputHeight, size_t blockSize);

extern PerceptionPtr imageFunctionToFlattenPerception(ImageFunctionPtr function);

};

#endif //!LBCPP_IMAGE_H_
