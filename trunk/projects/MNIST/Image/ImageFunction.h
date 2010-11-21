#ifndef LBCPP_IMAGE_FUNCTION_H_
# define LBCPP_IMAGE_FUNCTION_H_

# include <lbcpp/lbcpp.h>
# include "Image.h"

namespace lbcpp
{
  
class ImageFunction : public Function
{
public:
  ImageFunction(size_t inputWidth = 0, size_t inputHeight = 0) : width(inputWidth), height(inputHeight) {}
  
  virtual TypePtr getInputType() const
    {return imageClass;}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {jassert(inputType->inheritsFrom(imageClass)); return inputType;}
  
  virtual TypePtr getOutputImageType() const
    {return doubleType;}
  
  virtual size_t getOutputImageWidth() const
    {return width;}
  
  virtual size_t getOutputImageHeight() const
    {return height;}

protected:
  friend class ImageFunctionClass;
  
  size_t width;
  size_t height;
};

class IdentityImageFunction : public ImageFunction
{
public:
  IdentityImageFunction(size_t inputWidth, size_t inputHeight) : ImageFunction(inputWidth, inputHeight) {}
  IdentityImageFunction() {}
  
  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
    {return input;}

protected:
  friend class IdentityImageFunctionClass;
};

class AverageImageFunction : public ImageFunction
{
public:
  AverageImageFunction(size_t inputWidth, size_t inputHeight, size_t radius) : ImageFunction(inputWidth, inputHeight), radius(radius) {}
  AverageImageFunction() {}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    jassert(input.isObject());
    ImagePtr image = input.getObjectAndCast<Image>();
    jassert(image);
    
    size_t width = image->getWidth();
    size_t height = image->getHeight();
    ImagePtr res = new Image(width, height);
    for (size_t i = 0; i < width; ++i)
      for (size_t j = 0; i < height; ++j)
      {
        size_t startX = juce::jlimit(0, (int)width-1, (int)(i - radius));
        size_t endX   = juce::jlimit(0, (int)width-1, (int)(i + radius));
        size_t startY = juce::jlimit(0, (int)height-1, (int)(j - radius));
        size_t endY   = juce::jlimit(0, (int)height-1, (int)(j + radius));
        
        double sum = 0.0;
        for (size_t ii = startX; ii <= endX; ++ii)
          for (size_t jj = startY; jj <= endY; ++jj)
            sum += image->getValue(ii, jj);
        
        res->setValue(i, j, sum / ((endX - startX + 1) * (endY - startY + 1)));
      }

    return res;
  }

protected:
  friend class AverageImageFunctionClass;
  
  size_t radius;
};

class ReduceImageFunction : public  ImageFunction
{
public:
  ReduceImageFunction(size_t inputWidth, size_t inputHeight, size_t scaleFactor) : ImageFunction(inputWidth, inputHeight), scaleFactor(scaleFactor) {}
  ReduceImageFunction() {}

  virtual size_t getOutputImageWidth() const
    {return width / scaleFactor;}
  
  virtual size_t getOutputImageHeight() const
    {return height / scaleFactor;}
  
  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    jassert(input.isObject());
    ImagePtr image = input.getObjectAndCast<Image>();
    jassert(image);

    size_t width = image->getWidth();
    size_t height = image->getHeight();
    size_t scaledWidth = width / scaleFactor;
    size_t scaledHeight = height / scaleFactor;

    ImagePtr res = new Image(scaledWidth, scaledHeight);
    for (size_t i = width % scaleFactor, indexX = 0; i < width; i += scaleFactor, ++indexX)
      for (size_t j = height % scaleFactor, indexY = 0; j < height; j += scaleFactor, ++indexY)
        res->setValue(indexX, indexY, image->getValue(i, j));

    return res;
  }

protected:
  friend class ReduceImageFunctionClass;
  
  size_t scaleFactor;
};

class BinarizeImageFunction : public ImageFunction
{
public:
  BinarizeImageFunction(size_t inputWidth, size_t inputHeight, double threshold = 0.1) : ImageFunction(inputWidth, inputHeight), threshold(threshold)
    {jassert(0.0 <= threshold <= 1.0);}
  BinarizeImageFunction() {}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {jassert(inputType->inheritsFrom(imageClass)); return binaryImageClass;}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    jassert(input.isObject());
    ImagePtr image = input.getObjectAndCast<Image>();
    jassert(image);
    
    size_t width = image->getWidth();
    size_t height = image->getHeight();

    ImagePtr res = new BinaryImage(width, height);
    for (size_t i = 0; i < width; ++i)
      for (size_t j = 0; j < height; ++j)
        res->setValue(i, j, image->getValue(i, j) >= threshold);
    return res;
  }
  
protected:
  friend class BinarizeImageFunctionClass;
  
  double threshold;
};
  
class MinimumImageFunction : public ImageFunction
{
public:
  MinimumImageFunction(size_t inputWidth, size_t inputHeight) : ImageFunction(inputWidth, inputHeight) {}
  MinimumImageFunction() {}
  
  virtual size_t getOutputImageWidth() const
    {return width / 2;}
  
  virtual size_t getOutputImageHeight() const
    {return height / 2;}
  
  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    jassert(input.isObject());
    ImagePtr image = input.getObjectAndCast<Image>();
    jassert(image);
    
    ImagePtr res = new Image(width / 2, height / 2);
    for (size_t i = 0; i < width / 2; ++i)
      for (size_t j = 0; j < height / 2; ++j)
      {
        size_t x = 2 * i;
        size_t y = 2 * j;
        res->setValue(i, j, min(image->getValue(x, y),
                                image->getValue(x + 1, y),
                                image->getValue(x + 1, y + 1),
                                image->getValue(x, y + 1)));
      }
    return res;
  }

protected:
  friend class MinimumImageFunctionClass;
  
  double min(double a, double b, double c, double d) const
  {
    double res = a;
    if (b < res)
      res = b;
    if (c < res)
      res = c;
    if (d < res)
      res = d;
    return res;
  }
};
  
class MaximumImageFunction : public ImageFunction
{
public:
  MaximumImageFunction(size_t inputWidth, size_t inputHeight) : ImageFunction(inputWidth, inputHeight) {}
  MaximumImageFunction() {}
  
  virtual size_t getOutputImageWidth() const
    {return width / 2;}
  
  virtual size_t getOutputImageHeight() const
    {return height / 2;}
  
  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    jassert(input.isObject());
    ImagePtr image = input.getObjectAndCast<Image>();
    jassert(image);
    
    ImagePtr res = new Image(width / 2, height / 2);
    for (size_t i = 0; i < width / 2; ++i)
      for (size_t j = 0; j < height / 2; ++j)
      {
        size_t x = i * 2;
        size_t y = j * 2;
        res->setValue(i, j, max(image->getValue(x, y),
                                image->getValue(x + 1, y),
                                image->getValue(x + 1, y + 1),
                                image->getValue(x, y + 1)));
      }
    return res;
  }
  
protected:
  friend class MaximumImageFunctionClass;
  
  double max(double a, double b, double c, double d) const
  {
    double res = a;
    if (b > res)
      res = b;
    if (c > res)
      res = c;
    if (d > res)
      res = d;
    return res;
  }
};

};

#endif //!LBCPP_IMAGE_FUNCTION_H_
