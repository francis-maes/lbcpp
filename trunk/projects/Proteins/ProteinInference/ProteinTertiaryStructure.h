/*-----------------------------------------.---------------------------------.
| Filename: ProteinTertiaryStructure.h     | Protein Tertiary Structure      |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 16:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_PROTEIN_TERTIARY_STRUCTURE_H_
# define LBCPP_PROTEIN_INFERENCE_PROTEIN_TERTIARY_STRUCTURE_H_

# include "AminoAcidDictionary.h"
# include "../InferenceData/LabelSequence.h"

# include "../Geometry/DihedralAngle.h"
# include "../Geometry/Vector3.h"
# include "../Geometry/Matrix4.h"

namespace lbcpp
{

// generic
class CartesianCoordinatesSequence;
typedef ReferenceCountedObjectPtr<CartesianCoordinatesSequence> CartesianCoordinatesSequencePtr;
class FloryGeneralizedCoordinatesSequence;
typedef ReferenceCountedObjectPtr<FloryGeneralizedCoordinatesSequence> FloryGeneralizedCoordinatesSequencePtr;

// predeclarations
class ProteinDihedralAngles;
typedef ReferenceCountedObjectPtr<ProteinDihedralAngles> ProteinDihedralAnglesPtr;

class ProteinAtom;
typedef ReferenceCountedObjectPtr<ProteinAtom> ProteinAtomPtr;

class ProteinResidue;
typedef ReferenceCountedObjectPtr<ProteinResidue> ProteinResiduePtr;

class ProteinTertiaryStructure;
typedef ReferenceCountedObjectPtr<ProteinTertiaryStructure> ProteinTertiaryStructurePtr;

class CartesianCoordinatesSequence : public BuiltinVectorBasedSequence<Vector3>
{
public:
  typedef BuiltinVectorBasedSequence<Vector3> BaseClass;

  CartesianCoordinatesSequence(const String& name, size_t length)
    : BaseClass(name, length) {}
  CartesianCoordinatesSequence() {}

  Vector3 getPosition(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

  void setPosition(size_t index, const Vector3& position)
    {jassert(index < elements.size()); elements[index] = position;}
};

class FloryGeneralizedCoordinates : public Object
{
public:
  FloryGeneralizedCoordinates(double length, Angle theta, DihedralAngle phi)
    : length(length), theta(theta), phi(phi) {}
  FloryGeneralizedCoordinates() {}

  void multiplyMatrix(Matrix4& matrix, bool applyAngle, bool applyDihedralAngle)
  {
    matrix.translate(Vector3(length, 0.0, 0.0));
    if (applyDihedralAngle)
      matrix.rotateAroundXAxis(phi);
    if (applyAngle)
      matrix.rotateAroundZAxis(M_PI - theta);
  }

  virtual String toString() const
    {return T("(") + String(length, 2) + T(", ") + lbcpp::toString(theta) + T(", ") + lbcpp::toString(phi) + T(")");}

private:
  double length;
  Angle theta;
  DihedralAngle phi;
};

typedef ReferenceCountedObjectPtr<FloryGeneralizedCoordinates> FloryGeneralizedCoordinatesPtr;

class FloryGeneralizedCoordinatesSequence : public TypedObjectVectorBasedSequence<FloryGeneralizedCoordinates>
{
public:
  typedef TypedObjectVectorBasedSequence<FloryGeneralizedCoordinates> BaseClass;

  FloryGeneralizedCoordinatesSequence(const String& name, CartesianCoordinatesSequencePtr cartesianCoordinates)
    : BaseClass(name)
  {
    size_t n = cartesianCoordinates->size();
    jassert(n);
    if (n > 1)
    {
      resize(n - 1);
      Vector3 previousCoordinates = cartesianCoordinates->getPosition(0);
      Vector3 coordinates = cartesianCoordinates->getPosition(1);
      for (size_t i = 1; i < n; ++i)
      {
        Vector3 nextCoordinates;
        if (i < n - 1)
          nextCoordinates = cartesianCoordinates->getPosition(i + 1);

        Vector3 prev = previousCoordinates - coordinates;
        Vector3 next = nextCoordinates - coordinates;

        double length = prev.l2norm();
        Angle theta = i < n - 1 ? prev.angle(next) : 0.0;
        DihedralAngle phi = i > 1 && i < n - 1
          ? DihedralAngle(cartesianCoordinates->getPosition(i - 2), previousCoordinates, coordinates, nextCoordinates)
          : DihedralAngle(2 * M_PI);
        setCoordinates(i - 1, new FloryGeneralizedCoordinates(length, theta, phi));

        previousCoordinates = coordinates;
        coordinates = nextCoordinates;
      }
    }
  }

  FloryGeneralizedCoordinatesSequence(const String& name, size_t length = 0)
    : BaseClass(name, length) {}

  FloryGeneralizedCoordinatesSequence() {}  

  CartesianCoordinatesSequencePtr createCartesianCoordinates(const String& name, const Matrix4& initialMatrix = Matrix4::identity)
  {
    size_t n = size() + 1;

    CartesianCoordinatesSequencePtr res = new CartesianCoordinatesSequence(name, n);
    Matrix4 matrix(initialMatrix);
    res->setPosition(0, matrix.getTranslation());
    for (size_t i = 1; i < n; ++i)
    {
      getCoordinates(i - 1)->multiplyMatrix(matrix, i < n - 1, i < n - 2);
      res->setPosition(i, matrix.getTranslation());
    }
    return res;
  }

  void setCoordinates(size_t position, FloryGeneralizedCoordinatesPtr coordinates)
    {BaseClass::set(position, coordinates);}

  FloryGeneralizedCoordinatesPtr getCoordinates(size_t position) const
    {return BaseClass::get(position).dynamicCast<FloryGeneralizedCoordinates>();}
};

class ProteinDihedralAngles : public BuiltinVectorBasedSequence<DihedralAnglesPair>
{
public:
  typedef BuiltinVectorBasedSequence<DihedralAnglesPair> BaseClass;

  ProteinDihedralAngles(size_t length) : BaseClass(T("DihedralAngles"), length) {}
  ProteinDihedralAngles() {}
 
  static ProteinDihedralAnglesPtr createDihedralAngles(ProteinTertiaryStructurePtr tertiaryStructure);

  DihedralAnglesPair getAnglesPair(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

  DihedralAngle getPhi(size_t index) const
    {return getAnglesPair(index).first;}

  DihedralAngle getPsi(size_t index) const
    {return getAnglesPair(index).second;}

  void setAnglesPair(size_t index, DihedralAngle phi, DihedralAngle psi)
    {jassert(index < elements.size()); elements[index] = DihedralAnglesPair(phi, psi);}
};

class ProteinAtom : public NameableObject
{
public:
  ProteinAtom(const String& name, const String& elementSymbol, const Vector3& position = Vector3())
    : NameableObject(name), elementSymbol(elementSymbol), position(position), occupancy(0.0), temperatureFactor(0.0) {}
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

  double getDistanceBetweenAtoms(const String& name1, const String& name2) const;
  double getDistanceBetweenAtoms(const String& name1, ProteinResiduePtr residue2, const String& name2) const;

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

  static ProteinTertiaryStructurePtr createFromCAlphaTrace(LabelSequencePtr aminoAcidSequence, CartesianCoordinatesSequencePtr trace);
  static ProteinTertiaryStructurePtr createFromDihedralAngles(LabelSequencePtr aminoAcidSequence, ProteinDihedralAnglesPtr dihedralAngles);

  LabelSequencePtr createAminoAcidSequence() const;
  CartesianCoordinatesSequencePtr createCAlphaTrace() const;
  CartesianCoordinatesSequencePtr createCBetaTrace() const;

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

  ProteinResiduePtr getLastResidue() const
    {return residues.size() ? residues.back() : ProteinResiduePtr();}

  void setResidue(size_t index, ProteinResiduePtr residue)
    {jassert(index < residues.size()); residues[index] = residue;}

  void append(ProteinResiduePtr residue)
    {residues.push_back(residue);}

  bool hasOnlyCAlphaAtoms() const;
  bool isConsistent(String& failureReason) const;

private:
  std::vector<ProteinResiduePtr> residues;

  virtual bool load(InputStream& istr)
    {return Sequence::load(istr) && lbcpp::read(istr, residues);}

  virtual void save(OutputStream& ostr) const
    {Sequence::save(ostr); lbcpp::write(ostr, residues);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_PROTEIN_TERTIARY_STRUCTURE_H_
