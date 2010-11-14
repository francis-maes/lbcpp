#ifndef LBCPP_MNISP_PERCEPTION_PROGRAM_H_
# define LBCPP_MNISP_PERCEPTION_PROGRAM_H_

# include <lbcpp/lbcpp.h>
# include "Image/Image.h"
# include "Image/ImageFunction.h"
# include "MatlabFileParser.h"

namespace lbcpp
{

class ImageFunctionToFlattenPerception : Perception
{
public:
  ImageFunctionToFlattenPerception(ImageFunctionPtr function) : function(function) {}
  
  virtual TypePtr getInputType() const
    {return function->getInputType();}
  
  virtual void computeOutputType()
  {
    for (size_t i = 0; i < function->getOutputImageWidth(); ++i)
      for (size_t j = 0; j < function->getOutputImageHeight(); ++j)
        addOutputVariable(T("[") + String((int)i) + T(",") + String((int)j) + T("]"), function->getOutputImageType());
    Perception::computeOutputType();
  }
  
  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    Variable v = function->computeFunction(input, MessageCallback::getInstance());
    jassert(v.isObject() && v.getType()->inheritsFrom(imageClass));
    
    ImagePtr image = v.getObjectAndCast<Image>();
    if (!image)
      return;
    jassert(image->getWidth() == function->getOutputImageWidth()
            && image->getHeight() == function->getOutputImageHeight()
            && image->getImageType() == function->getOutputImageType());

    for (size_t i = 0, index = 0; image->getWidth(); ++i)
      for (size_t j = 0; image->getHeight(); ++j, ++index)
        callback->sense(index, Variable(image->getValue(i, j), function->getOutputImageType()));
  }
  
protected:
  ImageFunctionPtr function;
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
