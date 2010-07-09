/*-----------------------------------------.---------------------------------.
| Filename: CartesianPositionVector.cpp    | A sequence of (x,y,z) vectors   |
| Author  : Francis Maes                   |                                 |
| Started : 09/07/2010 13:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "CartesianPositionVector.h"
using namespace lbcpp;

void CartesianPositionVector::movePosition(size_t index, const Vector3& delta)
{
  Vector3 v = getPosition(index);
  jassert(v.exists());
  v += delta;
  setPosition(index, v);
}

void CartesianPositionVector::applyAffineTransform(const Matrix4& affineTransform)
{
  for (size_t i = 0; i < values.size(); ++i)
    if (values[i].exists())
      values[i] = affineTransform.transformAffine(values[i]);
}

Vector3 CartesianPositionVector::getGravityCenter() const
{
  // todo: cache
  Vector3 sum = 0;
  size_t count = 0;
  for (size_t i = 0; i < values.size(); ++i)
    if (values[i].exists())
    {
      sum += values[i];
      ++count;
    }
  return count ? sum / (double)count : sum;
}

void CartesianPositionVector::saveToXml(XmlElement* xml) const
{
  String str;

  for (size_t i = 0; i < values.size(); ++i)
  {
    if (values[i].exists())
      str += values[i].toString();
    else
      str += T("_");
    
    if (i < values.size() - 1)
      str += ' ';
  }

  xml->addTextElement(str);
  xml->setAttribute(T("size"), (int)values.size());
}

bool CartesianPositionVector::loadFromXml(XmlElement* xml, ErrorHandler& callback)
{
  int size = xml->getIntAttribute(T("size"), -1);
  if (size < 0)
  {
    callback.errorMessage(T("CartesianPositionVector::loadFromXml"), T("Invalid size: ") + String(size));
    return false;
  }
  values.resize(size, Vector3());

  String text = xml->getAllSubText();
  StringArray tokens;
  tokens.addTokens(text, T("() \t\r\n,"), NULL);

  std::vector<String> trueTokens;
  trueTokens.reserve(tokens.size() / 2);
  for (int i = 0; i < tokens.size(); ++i)
    if (tokens[i].isNotEmpty() && tokens[i] != T(" "))
      trueTokens.push_back(tokens[i]);
  
  size_t index = 0;
  for (size_t i = 0; i < trueTokens.size(); ++i)
  {
    if (index >= values.size())
    {
      callback.errorMessage(T("CartesianPositionVector::loadFromXml"), T("Too much tokens"));
      return false;
    }
    if (trueTokens[i] == T("_"))
      ++index;
    else 
    {
      if ((i >= trueTokens.size() - 2) || trueTokens[i + 1] == T("_") || trueTokens[i + 2] == T("_"))
      {
        callback.errorMessage(T("CartesianPositionVector::loadFromXml"),
          T("Invalid syntax: ") + trueTokens[i].quoted() + T(" ") + (i + 1 < trueTokens.size() ? trueTokens[i + 1].quoted() : T("N/A"))
                  + T(" ") + (i + 2 < trueTokens.size() ? trueTokens[i + 2].quoted() : T("N/A")));
        return false;
      }
      values[index++] = Vector3(trueTokens[i].getDoubleValue(), trueTokens[i + 1].getDoubleValue(), trueTokens[i + 2].getDoubleValue());
      i += 2;
    }
  }
  if (index < values.size())
  {
    callback.errorMessage(T("CartesianPositionVector::loadFromXml"), T("Too few tokens"));
    return false;
  }
  return true;
}

/////

ClassPtr lbcpp::vector3Class()
  {static TypeCache cache(T("Vector3Object")); return cache();}

ClassPtr lbcpp::cartesianPositionVectorClass()
  {static TypeCache cache(T("CartesianPositionVector")); return cache();}

void declareCartesianPositionVectorClasses()
{
  LBCPP_DECLARE_CLASS(Vector3Object, Object);
  LBCPP_DECLARE_CLASS(CartesianPositionVector, VariableContainer);
}