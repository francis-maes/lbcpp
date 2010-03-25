#include <lbcpp/lbcpp.h>
using namespace lbcpp;

class AminoAcid : public Object
{
public:
  enum Type
  {
    bla,
    fixme,
    unknown
  };
  
  AminoAcid(const AminoAcid& aa)
    : t(aa.t) {}
  AminoAcid(const Type& type = unknown)
    : t(type) {}
  
  // TODO: properties: one-letter code, three-letter code, hydrophoby, polarity, side chain, ....
  
  bool operator ==(const AminoAcid& aa) const
    {return t == aa.t;}
    
  bool operator !=(const AminoAcid& aa) const
    {return t != aa.t;}
    
  operator char() const
    {return (char)t;}

private:
  Type t;
};

class SequenceObject : public Object
{
public:
  virtual size_t getLength() const = 0;
  virtual ObjectPtr getObject(size_t position) const = 0;
};

class AminoAcidSequence : public SequenceObject
{
public:
  virtual std::string getName() const
    {return name;}

  virtual std::string toString() const // FASTA format
  {
    std::string res = "> " + name + "\n";
    for (size_t i = 0; i < sequence.size(); ++i)
      res += sequence[i];
    res += "\n";
    return res;
  }
  
  virtual size_t getLength() const
    {return sequence.size();}
  
  virtual ObjectPtr getObject(size_t position) const
    {return ObjectPtr(new AminoAcid(getAminoAcid(position)));}
  
  AminoAcid getAminoAcid(size_t position) const
    {assert(position < sequence.size()); return AminoAcid((AminoAcid::Type)sequence[position]);}
    
  void setAminoAcid(size_t position, const AminoAcid& aa)
    {assert(position < sequence.size()); sequence[position] = (char)aa;}

protected:
  virtual bool load(std::istream& istr)
  {
    return lbcpp::read(istr, name) && 
           lbcpp::read(istr, sequence);
  }
  
  virtual void save(std::ostream& ostr) const
  {
    lbcpp::write(ostr, name);
    lbcpp::write(ostr, sequence);
  }

private:
  std::string name;
  std::vector<char> sequence;
};

class SecondaryStructureSequence : public SequenceObject
{
public:
  virtual size_t getLength() const
    {return 0;}
  
  virtual ObjectPtr getObject(size_t position) const
    {return ObjectPtr();}
};

// Data: 

// AminoAcid,
// LabelSequence: AminoAcidSequence
// VectorSequence: PositionSpecificScoringMatrix
// LabelSequence: 3 state and 8 state SecondaryStructureSequence
// LabelSequence: 2 state SolventAccesibilitySequence
// ScalarSequence: regression SolventAccesibilitySequence
// ScalarMatrix: 2 state ResidueContactMap
// ScalarMatrix: regression ResidueDistanceMap
// VectorSequence: BackboneSequence
// Object: ThirdaryStructure

int main()
{
  return 0;
}