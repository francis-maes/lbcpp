#ifndef LBCPP_MNISP_PERCEPTION_PROGRAM_H_
# define LBCPP_MNISP_PERCEPTION_PROGRAM_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class ImageFlattenPerception : public Perception
{
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

extern PerceptionPtr imageFlattenPerception();
  
};

#endif //!LBCPP_MNISP_PERCEPTION_PROGRAM_H_
