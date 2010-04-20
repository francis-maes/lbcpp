/*-----------------------------------------.---------------------------------.
| Filename: ProteinTertiaryStructure.h     | Protein Tertiary Structure      |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 16:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_PROTEIN_TERTIARY_STRUCTURE_H_
# define LBCPP_PROTEIN_INFERENCE_PROTEIN_TERTIARY_STRUCTURE_H_

# include "../InferenceData/LabelSequence.h"
# include "AminoAcidDictionary.h"

namespace lbcpp
{

class Vector3
{
public:
  Vector3(double x, double y, double z) : x(x), y(y), z(z) {}
  Vector3() : x(0.0), y(0.0), z(0.0) {}

  String toString() const
    {return T("(") + lbcpp::toString(x) + T(", ") + lbcpp::toString(y) + T(", ") + lbcpp::toString(z) + T(")");}

  double getX() const
    {return x;}

  double getY() const
    {return y;}

  double getZ() const
    {return z;}

  void set(double x, double y, double z)
    {this->x = x; this->y = y; this->z = z;}

  Vector3 operator +(const Vector3& otherVector) const
    {return Vector3(x + otherVector.x, y + otherVector.y, z + otherVector.z);}

  Vector3 operator -(const Vector3& otherVector) const
    {return Vector3(x - otherVector.x, y - otherVector.y, z - otherVector.z);}

  Vector3 operator *(const double value) const
    {return Vector3(x * value, y * value, z * value);}

  Vector3 operator /(const double value) const
  {
    jassert(value);
    double inverseValue = 1.0 / value;
    return (*this) * inverseValue;
  }

  double dotProduct(const Vector3& otherVector) const
    {return x * otherVector.x + y * otherVector.y + z * otherVector.z;}

  double angle(const Vector3& otherVector) const
  {
    double n1 = sumOfSquares();
    double n2 = otherVector.sumOfSquares();
    if (!n1 || !n2)
      return 0.0;
    return acos(dotProduct(otherVector) / sqrt(n1 * n2));
  }

  Vector3 crossProduct(const Vector3& otherVector) const
  {
    return Vector3(y * otherVector.z - otherVector.y * z,
                    z * otherVector.x - otherVector.z * x,
                    x * otherVector.y - otherVector.x * y);
  }

  double sumOfSquares() const
    {return x * x + y *y + z * z;}

  double l2norm() const
    {return sqrt(sumOfSquares());}

  Vector3 normalized() const
  {
    double s = sumOfSquares();
    if (s == 1)
      return *this;
    else
    {
      s = 1.0 / sqrt(s);
      return (*this) * s;
    }
  }

private:
  double x, y, z;
};

template<>
struct Traits<Vector3>
{
  typedef bool Type;

  static inline String toString(const Vector3& value)
    {return value.toString();}

  static inline void write(OutputStream& ostr, const Vector3& value)
  {
    lbcpp::write(ostr, value.getX());
    lbcpp::write(ostr, value.getY());
    lbcpp::write(ostr, value.getZ());
  }

  static inline bool read(InputStream& istr, Vector3& res)
  {
    double x, y, z;
    if (!lbcpp::read(istr, x) || !lbcpp::read(istr, y) || !lbcpp::read(istr, z))
      return false;
    res.set(x, y, z);
    return true;
  }
};

// predeclarations
class ProteinCarbonTrace;
typedef ReferenceCountedObjectPtr<ProteinCarbonTrace> ProteinCarbonTracePtr;

class ProteinDihedralAngles;
typedef ReferenceCountedObjectPtr<ProteinDihedralAngles> ProteinDihedralAnglesPtr;

class ProteinAtom;
typedef ReferenceCountedObjectPtr<ProteinAtom> ProteinAtomPtr;

class ProteinResidue;
typedef ReferenceCountedObjectPtr<ProteinResidue> ProteinResiduePtr;

class ProteinTertiaryStructure;
typedef ReferenceCountedObjectPtr<ProteinTertiaryStructure> ProteinTertiaryStructurePtr;

/*
template<class ElementType>
class BuiltinVectorBasedSequence : public Sequence
{
public:
  typedef std::vector<ElementType> VectorType;

  virtual size_t size() const
    {return elements.size();}

protected:
  VectorType elements;
};
*/
class ProteinDihedralAngles : public Sequence
{
public:
  ProteinDihedralAngles(size_t length) : Sequence(T("DihedralAngles")), angles(length) {}
  ProteinDihedralAngles() {}
 
  static ProteinDihedralAnglesPtr createDihedralAngles(ProteinTertiaryStructurePtr tertiaryStructure);

  virtual size_t size() const
    {return angles.size();}

  virtual ObjectPtr get(size_t index) const
    {jassert(false); return ObjectPtr();}

  virtual FeatureGeneratorPtr elementFeatures(size_t position) const
    {return FeatureGeneratorPtr();}

  virtual FeatureGeneratorPtr sumFeatures(size_t begin, size_t end) const
    {return FeatureGeneratorPtr();}

private:
  struct ResidueInfo
  {
    ResidueInfo(ProteinResiduePtr previousResidue, ProteinResiduePtr residue, ProteinResiduePtr nextResidue);
    ResidueInfo() : phiAngle(0.0), psiAngle(0.0) {}

    double phiAngle;
    double psiAngle;
  };

  std::vector<ResidueInfo> angles;
};

class ProteinCarbonTrace : public Sequence
{
public:
  ProteinCarbonTrace(const String& name, size_t length)
    : Sequence(name), positions(length) {}
  ProteinCarbonTrace() {}

  static ProteinCarbonTracePtr createCAlphaTrace(ProteinTertiaryStructurePtr tertiaryStructure);
  static ProteinCarbonTracePtr createCBetaTrace(ProteinTertiaryStructurePtr tertiaryStructure);

  Vector3 getPosition(size_t index) const
    {jassert(index < positions.size()); return positions[index];}

  void setPosition(size_t index, const Vector3& position)
    {jassert(index < positions.size()); positions[index] = position;}

  virtual size_t size() const
    {return positions.size();}

  virtual ObjectPtr get(size_t index) const
    {return elementFeatures(index);}

  virtual FeatureGeneratorPtr elementFeatures(size_t position) const
    {return unitFeatureGenerator();}

  virtual FeatureGeneratorPtr sumFeatures(size_t begin, size_t end) const
    {return unitFeatureGenerator();}

protected:
  virtual bool load(InputStream& istr)
    {return Sequence::load(istr) && lbcpp::read(istr, positions);}

  virtual void save(OutputStream& ostr) const
    {Sequence::save(ostr); lbcpp::write(ostr, positions);}

private:
  std::vector<Vector3> positions;
};

class ProteinAtom : public NameableObject
{
public:
  ProteinAtom(const String& name, const String& elementSymbol)
    : NameableObject(name), elementSymbol(elementSymbol), occupancy(0.0), temperatureFactor(0.0) {}
  ProteinAtom() {}

  virtual String toString() const;

  String getElementSymbol() const
    {return elementSymbol;}

  const Vector3& getPosition() const
    {return position;}

  void setPosition(const Vector3& position)
    {this->position = position;}

  double getX() const
    {return position.getX();}

  double getY() const
    {return position.getY();}

  double getZ() const
    {return position.getZ();}

  void setOccupancy(double occupancy)
    {this->occupancy = occupancy;}

  double getOccupancy() const
    {return occupancy;}

  void setTemperatureFactor(double temperatureFactor)
    {this->temperatureFactor = temperatureFactor;}

  double getTemperatureFactor() const
    {return temperatureFactor;}

protected:
  String elementSymbol;
  Vector3 position;
  double occupancy;
  double temperatureFactor;

  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

class ProteinResidue : public Object
{
public:
  ProteinResidue(AminoAcidDictionary::Type aminoAcid)
    : aminoAcid(aminoAcid) {}
  ProteinResidue()
    : aminoAcid(AminoAcidDictionary::unknown) {}

  virtual String toString() const;

  virtual String getName() const
    {return AminoAcidDictionary::getThreeLettersCode(aminoAcid);}

  AminoAcidDictionary::Type getAminoAcid() const
    {return aminoAcid;}

  size_t getNumAtoms() const
    {return atoms.size();}

  ProteinAtomPtr getAtom(size_t index) const
    {jassert(index < atoms.size()); return atoms[index];}

  void addAtom(ProteinAtomPtr atom)
    {atoms.push_back(atom);}

  // central atom (back bone, first atom of the side chain)
  ProteinAtomPtr getCAlphaAtom() const
    {return findAtomByName(T("CA"));}

  // second carbon of the side chain
  ProteinAtomPtr getCBetaAtom() const
    {return findAtomByName(T("CB"));}

  // back bone
  ProteinAtomPtr getNitrogenAtom() const
    {return findAtomByName(T("N"));}

  // back bone
  ProteinAtomPtr getCarbonAtom() const
    {return findAtomByName(T("C"));}

  ProteinAtomPtr findAtomByName(const String& name) const;

protected:
  AminoAcidDictionary::Type aminoAcid;
  std::vector<ProteinAtomPtr> atoms;

  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

class ProteinTertiaryStructure : public Sequence
{
public:
  ProteinTertiaryStructure(size_t numResidues);
  ProteinTertiaryStructure() {}

  static ProteinTertiaryStructurePtr createFromCAlphaTrace(LabelSequencePtr aminoAcidSequence, ProteinCarbonTracePtr trace);

  virtual size_t size() const
    {return residues.size();}

  virtual ObjectPtr get(size_t index) const
    {jassert(index < residues.size()); return residues[index];}

  virtual void resize(size_t newSize)
    {residues.resize(newSize);}

  virtual void set(size_t index, ObjectPtr object);

  virtual FeatureGeneratorPtr elementFeatures(size_t position) const
    {return FeatureGeneratorPtr();} // todo
  virtual FeatureGeneratorPtr sumFeatures(size_t begin, size_t end) const
    {return FeatureGeneratorPtr();} // todo

  ProteinResiduePtr getResidue(size_t index) const
    {jassert(index < residues.size()); return residues[index];}

  void setResidue(size_t index, ProteinResiduePtr residue)
    {jassert(index < residues.size()); residues[index] = residue;}

  void append(ProteinResiduePtr residue)
    {residues.push_back(residue);}

private:
  std::vector<ProteinResiduePtr> residues;

  virtual bool load(InputStream& istr)
    {return Sequence::load(istr) && lbcpp::read(istr, residues);}

  virtual void save(OutputStream& ostr) const
    {Sequence::save(ostr); lbcpp::write(ostr, residues);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_PROTEIN_TERTIARY_STRUCTURE_H_
