/*-----------------------------------------.---------------------------------.
| Filename: Atom.h                         | Atom                            |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 15:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_ATOM_H_
# define LBCPP_PROTEIN_ATOM_H_

# include "../../Geometry/Vector3.h"

namespace lbcpp
{

class Atom : public NameableObject
{
public:
  Atom(const String& name, const String& elementSymbol, const impl::Vector3& position = impl::Vector3())
    : NameableObject(name), elementSymbol(elementSymbol), position(position), occupancy(0.0), temperatureFactor(0.0) {}
  Atom() {}

  String getElementSymbol() const
    {return elementSymbol;}

  const impl::Vector3& getPosition() const
    {return position;}
  
  impl::Vector3& getPosition()
    {return position;}

  void setPosition(const impl::Vector3& position)
    {this->position = position;}

  double getX() const
    {return position.getX();}

  void setX(double value)
    {position.setX(value);}

  double getY() const
    {return position.getY();}

  void setY(double value)
    {position.setY(value);}

  double getZ() const
    {return position.getZ();}

  void setZ(double value)
    {position.setZ(value);}

  void setOccupancy(double occupancy)
    {this->occupancy = occupancy;}

  double getOccupancy() const
    {return occupancy;}

  void setTemperatureFactor(double temperatureFactor)
    {this->temperatureFactor = temperatureFactor;}

  double getTemperatureFactor() const
    {return temperatureFactor;}

  /*
  ** Object
  */
  virtual String toString() const;
  virtual void saveToXml(XmlElement* xml) const;
  virtual bool loadFromXml(XmlElement* xml, ErrorHandler& callback);

protected:
  String elementSymbol;
  impl::Vector3 position;
  double occupancy;
  double temperatureFactor;
};

typedef ReferenceCountedObjectPtr<Atom> AtomPtr;

extern ClassPtr atomClass();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_ATOM_H_
