#ifndef LBCPP_MNIST_IMAGE_H_
# define LBCPP_MNIST_IMAGE_H_

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
  
};

#endif //!LBCPP_MNIST_IMAGE_H_
