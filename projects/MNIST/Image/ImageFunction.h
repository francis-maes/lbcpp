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
    {return probabilityType;}
  
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

  virtual TypePtr getOutputImageType() const
    {return booleanType;}

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
  MinimumImageFunction(size_t inputWidth, size_t inputHeight, size_t blockSize) : ImageFunction(inputWidth, inputHeight), blockSize(blockSize) {}
  MinimumImageFunction() : blockSize(0) {}
  
  virtual size_t getOutputImageWidth() const
    {return (size_t)ceil(width / (double)blockSize);}
  
  virtual size_t getOutputImageHeight() const
    {return (size_t)ceil(height / (double)blockSize);}
  
  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    jassert(input.isObject());
    ImagePtr image = input.getObjectAndCast<Image>();
    jassert(image);
    
    size_t scaledWidth = (size_t)ceil(width / (double)blockSize);
    size_t scaledHeight = (size_t)ceil(height / (double)blockSize);
    ImagePtr res = new Image(scaledWidth, scaledHeight);
    for (size_t i = 0; i < scaledWidth; ++i)
      for (size_t j = 0; j < scaledHeight; ++j)
      {
        size_t startX = blockSize * i;
        size_t startY = blockSize * j;
        size_t endX = juce::jmin((int)width, (int)(startX + blockSize));
        size_t endY = juce::jmin((int)height, (int)(startY + blockSize));
        res->setValue(i, j, squareMin(image, startX, startY, endX, endY));
      }
    return res;
  }
  
protected:
  friend class MinimumImageFunctionClass;
  
  size_t blockSize;
  
  double squareMin(ImagePtr image, size_t startX, size_t startY, size_t endX, size_t endY) const
  {
    double res = DBL_MAX;
    for (size_t i = startX; i < endX; ++i)
      for (size_t j = startY; j < endY; ++j)
        if (image->getValue(i, j) < res)
          res = image->getValue(i, j);
    return res;
  }
};
  
class MaximumImageFunction : public ImageFunction
{
public:
  MaximumImageFunction(size_t inputWidth, size_t inputHeight, size_t blockSize) : ImageFunction(inputWidth, inputHeight), blockSize(blockSize) {}
  MaximumImageFunction() : blockSize(0) {}
  
  virtual size_t getOutputImageWidth() const
    {return (size_t)ceil(width / blockSize);}
  
  virtual size_t getOutputImageHeight() const
    {return (size_t)ceil(height / 2);}
  
  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    jassert(input.isObject());
    ImagePtr image = input.getObjectAndCast<Image>();
    jassert(image);
    
    size_t scaledWidth = (size_t)ceil(width / blockSize);
    size_t scaledHeight = (size_t)ceil(height / blockSize);
    ImagePtr res = new Image(scaledWidth, scaledHeight);
    for (size_t i = 0; i < scaledWidth; ++i)
      for (size_t j = 0; j < scaledHeight; ++j)
      {
        size_t startX = i * blockSize;
        size_t startY = j * blockSize;
        size_t endX = juce::jmin((int)width, (int)(startX + blockSize));
        size_t endY = juce::jmin((int)height, (int)(startY + blockSize));
        res->setValue(i, j, squareMax(image, startX, startY, endX, endY));
      }
    return res;
  }
  
protected:
  friend class MaximumImageFunctionClass;
  
  size_t blockSize;
  
  double squareMax(ImagePtr image, size_t startX, size_t startY, size_t endX, size_t endY) const
  {
    double res = -DBL_MAX;
    for (size_t i = startX; i < endX; ++i)
      for (size_t j = startY; j < endY; ++j)
        if (image->getValue(i, j) > res)
          res = image->getValue(i, j);
    return res;
  }
};

};

#endif //!LBCPP_IMAGE_FUNCTION_H_
