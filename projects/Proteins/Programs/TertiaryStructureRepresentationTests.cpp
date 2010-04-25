/*-----------------------------------------.---------------------------------.
| Filename: TertiaryStructureRepresen...cpp| Test conversions between ts repr|
| Author  : Francis Maes                   |                                 |
| Started : 20/04/2010 18:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../Protein/Protein.h"
#include "../Protein/AminoAcidDictionary.h"
#include "../Protein/SecondaryStructureDictionary.h"
using namespace lbcpp;

extern void declareProteinClasses();

// returns the matrix (rotation + translation) to transform points1 into points2
Matrix4 superposeStructures(const std::vector< std::pair<Vector3, Vector3> >& pointPairs)
{
  size_t n = pointPairs.size();
  jassert(n);
  double invN = 1.0 / (double)n;

  // compute centroids
  Vector3 centroid1(0.0), centroid2(0.0);
  for (size_t i = 0; i < n; ++i)
  {
    centroid1 += pointPairs[i].first;
    centroid2 += pointPairs[i].second;
  }
  centroid1 *= invN;
  centroid2 *= invN;

  // compute correlation matrix
  Matrix3 correlationMatrix = Matrix3::zero;
  for (size_t i = 0; i < n; ++i)
    correlationMatrix.addCorrelation(pointPairs[i].first - centroid1, pointPairs[i].second - centroid2);

  // make SVD decomposition
  Matrix3 u, v;
  Vector3 diag;
  bool ok = correlationMatrix.makeSVDDecomposition(u, diag, v);
  jassert(ok);
  if (!ok)
    return Matrix4::zero;

  // compute optimal rotation matrix
  Matrix3 rotation = v * u.transposed();

  // compute optimal translation
  Vector3 translation(0.0);
  for (size_t i = 0; i < n; ++i)
  {
    translation += pointPairs[i].second;
    Vector3 p = rotation.transformAffine(pointPairs[i].first);
    translation -= p;
  }
  translation *= invN;

  return Matrix4(rotation, translation);
}

Matrix4 superposeCAlphaAtoms(ProteinTertiaryStructurePtr structure1, ProteinTertiaryStructurePtr structure2)
{
  jassert(structure1 && structure2);
  size_t n = structure1->size();
  jassert(n && structure2->size() == n);

  std::vector< std::pair<Vector3, Vector3> > pointPairs;
  pointPairs.reserve(n);
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResiduePtr residue1 = structure1->getResidue(i);
    ProteinResiduePtr residue2 = structure2->getResidue(i);
    if (!residue1 || !residue2)
      continue;
    Vector3 position1 = residue1->getAtomPosition(T("CA"));
    Vector3 position2 = residue2->getAtomPosition(T("CA"));
    if (!position1.exists() || !position2.exists())
      continue;
    
    pointPairs.push_back(std::make_pair(position1, position2));
  }
  return superposeStructures(pointPairs);
}

int main()
{
  declareProteinClasses();

  File proteinDatabase(T("C:\\Projets\\LBC++\\projects\\temp\\SmallPDB\\protein"));
  ObjectStreamPtr proteinsStream = directoryObjectStream(proteinDatabase, T("*.protein"));
  ObjectContainerPtr proteins = proteinsStream->load(1)->randomize();
  std::cout << proteins->size() << " proteins." << std::endl;
  if (!proteins)
    return 1;

  File ramachadranPlotFile(T("C:\\Projets\\LBC++\\projects\\temp\\ramac.txt"));
  ramachadranPlotFile.deleteRecursively();
  OutputStream* ramachadranPlot = ramachadranPlotFile.createOutputStream();
  jassert(ramachadranPlot);

  static ScalarVariableStatistics nCalphaLength(T("N--CA length"));
  static ScalarVariableStatistics calphaCLength(T("CA--C length")); 
  static ScalarVariableStatistics cnLength(T("C--N length")); 
  static ScalarVariableStatistics calphaCalphaLength(T("CA--CA length")); 

  static ScalarVariableStatistics calphaAngle(T("N--CA--C angle"));
  static ScalarVariableStatistics carbonAngle(T("CA--C--N' angle"));
  static ScalarVariableStatistics nitrogenAngle(T("C--N'--CA' angle"));

  for (size_t index = 0; index < proteins->size(); ++index)
  {
    ProteinPtr protein = proteins->getAndCast<Protein>(index);
    std::cout << "Protein " << protein->getName() << std::endl;

    ProteinTertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
    jassert(tertiaryStructure);
    if (tertiaryStructure->hasOnlyCAlphaAtoms())
      continue;

    LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
    size_t n = aminoAcidSequence->size();

    ProteinBackboneBondSequencePtr backbone = tertiaryStructure->createBackbone();
    ProteinTertiaryStructurePtr tertiaryStructure2 = ProteinTertiaryStructure::createFromBackbone(aminoAcidSequence, backbone);

    Matrix4 matrix = superposeCAlphaAtoms(tertiaryStructure2, tertiaryStructure);
    std::cout << "Superposition matrix: " << std::endl << matrix.toString() << std::endl;
    double fabsError = 0.0, rmsError = 0.0;
    size_t count = 0;
    for (size_t i = 0; i < n; ++i)
    {
      ProteinResiduePtr residue1 = tertiaryStructure->getResidue(i);
      ProteinResiduePtr residue2 = tertiaryStructure2->getResidue(i);
      if (!residue1 || !residue2)
        continue;
      Vector3 position1 = residue1->getAtomPosition(T("CA"));
      Vector3 position2 = residue2->getAtomPosition(T("CA"));
      if (!position1.exists() || !position2.exists())
        continue;
      
      double delta = (matrix.transformAffine(position2) - position1).l2norm();
      fabsError += fabs(delta);
      rmsError += delta * delta;
      ++count;
    }
    jassert(count);
    fabsError /= (double)count;
    rmsError /= (double)count;
    std::cout << "TS: fabs error = " << fabsError << " rmse = " << sqrt(rmsError) << std::endl;
    break;

    ProteinBackboneBondSequencePtr backbone2 = tertiaryStructure2->createBackbone();
    for (size_t i = n - 20; i < n; ++i)
    {
      std::cout << (i+1) << "Correct: " << backbone->getBond(i)->toString() << std::endl
        << " Reconstructed: " << backbone2->getBond(i)->toString() << std::endl;
    }

    protein->setObject(tertiaryStructure2);
    protein->saveToPDBFile(File(T("C:/Projets/LBC++/projects/temp/pouet.pdb")));
    break;

    for (size_t i = 0; i < tertiaryStructure->size(); ++i)
    {
      ProteinResiduePtr residue = tertiaryStructure->getResidue(i);

      Vector3 nitrogen = residue->getNitrogenAtom()->getPosition();
      Vector3 calpha = residue->getCAlphaAtom()->getPosition();
      Vector3 carbon = residue->getCarbonAtom()->getPosition();

      nCalphaLength.push((calpha - nitrogen).l2norm());
      calphaCLength.push((carbon - calpha).l2norm());
      ProteinResiduePtr nextResidue = i < tertiaryStructure->size() - 1 ? tertiaryStructure->getResidue(i + 1) : ProteinResiduePtr();
      if (nextResidue)
      {
        Vector3 nextNitrogen = nextResidue->getNitrogenAtom()->getPosition();
        cnLength.push((nextNitrogen - carbon).l2norm());

        Vector3 nextCalpha = nextResidue->getCAlphaAtom()->getPosition();
        calphaCalphaLength.push((nextCalpha - calpha).l2norm());

        carbonAngle.push((carbon - calpha).angle(nextNitrogen - carbon));
        nitrogenAngle.push((nextNitrogen - carbon).angle(nextCalpha - nextNitrogen));
      }

      calphaAngle.push((calpha - nitrogen).angle(carbon - calpha));
    }

    //for (size_t i = 1; i < dihedralAngles->size() - 1; ++i)
    //  (*ramachadranPlot) << lbcpp::toString(dihedralAngles->getPhi(i)) << " " << lbcpp::toString(dihedralAngles->getPsi(i)) << "\n";
  }

  std::cout << nCalphaLength.toString() << std::endl;
  std::cout << calphaCLength.toString() << std::endl;
  std::cout << cnLength.toString() << std::endl;
  std::cout << calphaCalphaLength.toString() << std::endl;
  std::cout << calphaAngle.toString() << std::endl;
  std::cout << carbonAngle.toString() << std::endl;
  std::cout << nitrogenAngle.toString() << std::endl;
  delete ramachadranPlot; 
  return 0;
}
