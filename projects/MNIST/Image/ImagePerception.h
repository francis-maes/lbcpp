#ifndef LBCPP_IMAGE_PERCEPTION_H_
# define LBCPP_IMAGE_PERCEPTION_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class ImageFunctionToFlattenPerception : public Perception
{
public:
  ImageFunctionToFlattenPerception(ImageFunctionPtr function) : function(function) {}
  ImageFunctionToFlattenPerception() {}
  
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
    
    for (size_t i = 0, index = 0; i < image->getWidth(); ++i)
      for (size_t j = 0; j < image->getHeight(); ++j, ++index)
        if (function->getOutputImageType()->inheritsFrom(booleanType))
          callback->sense(index, Variable(image->getValue(i, j) == 1.0, function->getOutputImageType()));
        else
          callback->sense(index, Variable(image->getValue(i, j), function->getOutputImageType()));
  }
  
protected:
  friend class ImageFunctionToFlattenPerceptionClass;
  
  ImageFunctionPtr function;
};

};

#endif //!LBCPP_IMAGE_PERCEPTION_H_
