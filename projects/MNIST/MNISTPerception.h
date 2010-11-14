#ifndef LBCPP_MNISP_PERCEPTION_PROGRAM_H_
# define LBCPP_MNISP_PERCEPTION_PROGRAM_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

extern ClassPtr imageClass;
extern ClassPtr binaryImageClass;
  
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
    {return doubleType;}

protected:
  friend class ImageClass;
  
  size_t width;
  size_t height;
  std::vector<double> pixels;

  size_t getIndex(size_t x, size_t y) const
    {jassert(x < width && y < height); return x * width + height;}
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
  
class ImageFunction : public Function
{
public:
  virtual TypePtr getInputType() const
    {return imageClass;}
  
  virtual TypePtr getOutputImageType() const {return doubleType;}
  // average
  // binarization
  // resize
  
  // getOutputWidth
  // getOutputHeight
  // getOutputType
  
};

class AverageImageFunction : public Function
{
public:
  AverageImageFunction(size_t radius) : radius(radius) {}

  virtual TypePtr getInputType() const
    {return imageClass;}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
  {
    jassert(inputType->inheritsFrom(imageClass));
    return inputType;
  }

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
  size_t radius;
};

class ReduceImageFunction : public  Function
{
public:
  ReduceImageFunction(size_t scaleFactor) : scaleFactor(scaleFactor) {}
  
  virtual TypePtr getInputType() const
    {return imageClass;}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
  {
    jassert(inputType->inheritsFrom(imageClass));
    return inputType;
  }
  
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
  size_t scaleFactor;
};

class BinarizeImageFunction : public Function
{
public:
  BinarizeImageFunction(double threshold = 0.1) : threshold(threshold)
    {jassert(0.0 <= threshold <= 1.0);}
  
  virtual TypePtr getInputType() const
    {return imageClass;}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {jassert(inputType->inheritsFrom(imageClass)); return binaryImageClass;}

  virtual void computeFunction(const Variable& input, PerceptionCallbackPtr callback) const
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
  }
  
protected:
  double threshold;
};

class ImageFlattenPerception : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return mnistImageClass;}
  
  virtual void computeOutputType()
  {
    for (size_t i = 0; i < MNISTImage::numPixels; ++i)
      addOutputVariable(String((int)i), probabilityType);
    Perception::computeOutputType();
  }
  
  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input.isObject());
    MNISTImagePtr image = input.getObjectAndCast<MNISTImage>();
    jassert(image);
    
    const std::vector<double>& pixels = image->getPixels();
    for (size_t i = 0; i < pixels.size(); ++i)
      callback->sense(i, Variable(pixels[i], probabilityType));
  }
};
  
class BinarizeImagePerception : public Perception
{
public:
  BinarizeImagePerception(double threshold = 0.1) : threshold(threshold)
    {jassert(0.0 <= threshold <= 1.0);}
  
  virtual TypePtr getInputType() const
    {return mnistImageClass;}
  
  virtual void computeOutputType()
  {
    for (size_t i = 0; i < MNISTImage::numPixels; ++i)
      addOutputVariable(String((int)i), booleanType);
    Perception::computeOutputType();
  }
  
  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input.isObject());
    MNISTImagePtr image = input.getObjectAndCast<MNISTImage>();
    jassert(image);
    
    const std::vector<double>& pixels = image->getPixels();
    for (size_t i = 0; i < pixels.size(); ++i)
      callback->sense(i, Variable((pixels[i] <= threshold), booleanType));
  }

protected:
  friend class BinarizeImagePerceptionClass;
  
  double threshold;
};

extern PerceptionPtr imageFlattenPerception();
extern PerceptionPtr binarizeImagePerception(double threshold);
  
};

#endif //!LBCPP_MNISP_PERCEPTION_PROGRAM_H_
