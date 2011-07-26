/*-----------------------------------------.---------------------------------.
| Filename: Protein.cpp                    | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "Protein.h"
#include "Formats/PDBFileParser.h"
#include "Formats/PDBFileGenerator.h"
#include "Formats/FASTAFileParser.h"
#include "Formats/FASTAFileGenerator.h"
#include <lbcpp/Distribution/DiscreteDistribution.h>

using namespace lbcpp;

ProteinPtr Protein::createFromPDB(ExecutionContext& context, const File& pdbFile, bool beTolerant)
{
  ReferenceCountedObjectPtr<PDBFileParser> parser(new PDBFileParser(context, pdbFile, beTolerant));
  if (!parser->next().exists())
    return ProteinPtr();
  
  std::vector<ProteinPtr> proteins = parser->getAllChains();
  jassert(proteins.size());
  ProteinPtr res = proteins[0];
  if (proteins.size() > 1)
  {
    size_t chainSize = proteins[0]->getLength();
    for (size_t i = 1; i < proteins.size(); ++i)
      if (proteins[i]->getLength() != chainSize)
      {
        for (size_t j = 0; j < proteins.size(); ++j)
          std::cerr << "Chain Size: " << proteins[j]->getLength() << std::endl;
        context.errorCallback(T("ProteinObject::createFromPDB"), T("This file contains chains of different size, I do not know which one to choose"));
        return ProteinPtr();
      }
  }
  
  VectorPtr primaryStructure = res->getPrimaryStructure();
  jassert(primaryStructure);
  TertiaryStructurePtr tertiaryStructure = res->getTertiaryStructure();
  jassert(tertiaryStructure && tertiaryStructure->getNumResidues() == primaryStructure->getNumElements());
  return res;
}

ProteinPtr Protein::createFromPDB(ExecutionContext& context, const String& pdbString,
		bool beTolerant)
{
	const char* tab0 = pdbString.toUTF8();
	juce::MemoryInputStream* mb = new juce::MemoryInputStream((const void*) tab0,
			pdbString.length(), true);

	ReferenceCountedObjectPtr<PDBFileParser> parser(new PDBFileParser(context, mb, beTolerant));

	if (!parser->next().exists())
		return ProteinPtr();

	std::vector<ProteinPtr> proteins = parser->getAllChains();
	jassert(proteins.size());
	ProteinPtr res = proteins[0];

	if (proteins.size() > 1)
	{
		size_t chainSize = proteins[0]->getLength();
		for (size_t i = 1; i < proteins.size(); ++i)
			if (proteins[i]->getLength() != chainSize)
			{
				for (size_t j = 0; j < proteins.size(); ++j)
					std::cerr << "Chain Size: " << proteins[j]->getLength() << std::endl;
				context.errorCallback(
						T("ProteinObject::createFromPDB"),
						T("This file contains chains of different size, I do not know which one to choose"));
				return ProteinPtr();
			}
	}

	VectorPtr primaryStructure = res->getPrimaryStructure();
	jassert(primaryStructure);
	TertiaryStructurePtr tertiaryStructure = res->getTertiaryStructure();
	jassert(tertiaryStructure && tertiaryStructure->getNumResidues() == primaryStructure->getNumElements());
	return res;
}

ProteinPtr Protein::createFromXml(ExecutionContext& context, const File& file)
  {return Variable::createFromFile(context, file).getObjectAndCast<Protein>(context);}

ProteinPtr Protein::createFromFASTA(ExecutionContext& context, const File& file)
  {return StreamPtr(new FASTAFileParser(context, file))->next().getObjectAndCast<Protein>(context);}

void Protein::saveToPDBFile(ExecutionContext& context, const File& pdbFile) const
  {ConsumerPtr(new PDBFileGenerator(context, pdbFile))->consume(context, refCountedPointerFromThis(this));}

void Protein::saveToXmlFile(ExecutionContext& context, const File& xmlFile) const
  {Object::saveToFile(context, xmlFile);}

void Protein::saveToFASTAFile(ExecutionContext& context, const File& fastaFile) const
  {ConsumerPtr(new FASTAFileGenerator(context, fastaFile))->consume(context, refCountedPointerFromThis(this));}

ContainerPtr Protein::loadProteinsFromDirectory(ExecutionContext& context, const File& directory, size_t maxCount, const String& workUnitName)
{
  return directoryFileStream(context, directory, T("*.xml"))->load(maxCount, false)
      ->apply(context, loadFromFileFunction(proteinClass), Container::parallelApply, workUnitName)
      ->randomize();
}

ContainerPtr Protein::loadProteinsFromDirectoryPair(ExecutionContext& context, const File& inputDirectory, const File& supervisionDirectory, size_t maxCount, const String& workUnitName)
{
  if (inputDirectory.exists())
    return directoryPairFileStream(context, inputDirectory, supervisionDirectory, T("*.xml"))->load(maxCount, false)
      ->apply(context, loadFromFilePairFunction(proteinClass, proteinClass), Container::parallelApply, workUnitName)->randomize();
  else
    return directoryFileStream(context, supervisionDirectory, T("*.xml"))->load(maxCount, false)
      ->apply(context, composeFunction(loadFromFileFunction(proteinClass), proteinToInputOutputPairFunction(true)), Container::parallelApply, workUnitName)
      ->randomize();
}

void Protein::setPrimaryStructure(VectorPtr primaryStructure)
{
  this->primaryStructure = primaryStructure;

  size_t n = primaryStructure->getNumElements();
  cysteinInvIndices.clear();
  cysteinInvIndices.resize(n, -1);
  cysteinIndices.clear();
  cysteinIndices.reserve(n / 20);

  for (size_t i = 0; i < n; ++i)
    if ((AminoAcidType)primaryStructure->getElement(i).getInteger() == cysteine)
    {
      cysteinInvIndices[i] = (int)cysteinIndices.size();
      cysteinIndices.push_back(i);
    }
}

bool Protein::loadFromXml(XmlImporter& importer)
{
  if (!Object::loadFromXml(importer))
    return false;
  if (primaryStructure)
    setPrimaryStructure(primaryStructure); // precompute cysteins info
  return true;
}

void Protein::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  Object::clone(context, target);
  const ProteinPtr& targetProtein = target.staticCast<Protein>();
  targetProtein->cysteinIndices = cysteinIndices;
  targetProtein->cysteinInvIndices = cysteinInvIndices;
}

void Protein::setPrimaryStructure(const String& primaryStructure)
{
  jassert(!this->primaryStructure);
  size_t n = primaryStructure.length();
  VectorPtr v = vector(aminoAcidTypeEnumeration, n);
  for (size_t i = 0; i < n; ++i)
    v->setElement(i, AminoAcid::fromOneLetterCode(primaryStructure[(int)i]));
  setPrimaryStructure(v);
}

void Protein::setContactMap(SymmetricMatrixPtr contactMap, double threshold, bool betweenCBetaAtoms)
{
  jassert(threshold == 8.0);
  if (betweenCBetaAtoms)
    contactMap8Cb = contactMap;
  else
    contactMap8Ca = contactMap;
}

void Protein::setDistanceMap(SymmetricMatrixPtr distanceMap, bool betweenCBetaAtoms)
{
  if (betweenCBetaAtoms)
    distanceMapCb = distanceMap;
  else
    distanceMapCa = distanceMap;
}

Variable Protein::getTargetOrComputeIfMissing(ExecutionContext& context, size_t index) const
{
  // skip base class variables
  size_t baseClassVariables = nameableObjectClass->getNumMemberVariables();
  if (index < baseClassVariables)
    return String::empty;
  index -= baseClassVariables;

  switch (index)
  {
  case 0: return getPrimaryStructure();
  case 1: return getPositionSpecificScoringMatrix();
  case 2: return getCysteinBondingProperty(context);
  case 3: return getSecondaryStructure();
  case 4: return getDSSPSecondaryStructure();
  case 5: return getStructuralAlphabetSequence();
  case 6: return getSolventAccessibility();
  case 7: return getSolventAccessibilityAt20p();
  case 8: return getDisorderRegions();
  case 9: return getCysteinBondingStates(context);
  case 10: return getContactMap(context, 8, false);
  case 11: return getContactMap(context, 8, true);
  case 12: return getDistanceMap(context, false);
  case 13: return getDistanceMap(context, true);
  case 14: return getDisulfideBonds(context);
  case 15: return getFullDisulfideBonds(context);
  case 16: return getCAlphaTrace();
  case 17: return getTertiaryStructure();
  default: jassert(false); return Variable();
  }
}

/*
** Compute Missing Targets
*/

Variable Protein::getCysteinBondingProperty(ExecutionContext& context) const
{
  if (!cysteinIndices.size())
    return Variable::missingValue(sparseDoubleVectorClass(cysteinBondingPropertyElementEnumeration, probabilityType));

  if (!cysteinBondingProperty)
  {
    DoubleVectorPtr bondingStates = getCysteinBondingStates(context);
    if (bondingStates)
      const_cast<Protein* >(this)->cysteinBondingProperty = computeCysteinBondingProperty(bondingStates);
  }
  return cysteinBondingProperty;
}


ContainerPtr Protein::getSecondaryStructure() const
{
  if (!secondaryStructure && dsspSecondaryStructure)
    const_cast<Protein* >(this)->secondaryStructure = computeSecondaryStructureFromDSSPSecondaryStructure(dsspSecondaryStructure);
  return secondaryStructure;
}

DoubleVectorPtr Protein::getSolventAccessibilityAt20p() const
{
  if (!solventAccessibilityAt20p && solventAccessibility)
    const_cast<Protein* >(this)->solventAccessibilityAt20p = computeBinarySolventAccessibilityFromSolventAccessibility(solventAccessibility, 0.2);
  return solventAccessibilityAt20p;
}

DoubleVectorPtr Protein::getDisorderRegions() const
{
  if (!disorderRegions && tertiaryStructure)
    const_cast<Protein* >(this)->disorderRegions = computeDisorderRegionsFromTertiaryStructure(tertiaryStructure);
  return disorderRegions;
}

ContainerPtr Protein::getStructuralAlphabetSequence() const
{
  if (!structuralAlphabetSequence)
  {
    CartesianPositionVectorPtr calphaTrace = getCAlphaTrace();
    if (calphaTrace)
      const_cast<Protein* >(this)->structuralAlphabetSequence = computeStructuralAlphabetSequenceFromCAlphaTrace(calphaTrace);
  }
  return structuralAlphabetSequence;
}

SymmetricMatrixPtr Protein::getDistanceMap(ExecutionContext& context, bool betweenCBetaAtoms) const
{
  if (betweenCBetaAtoms)
  {
    if (!distanceMapCb && tertiaryStructure && tertiaryStructure->hasBackboneAndCBetaAtoms())
      const_cast<Protein* >(this)->distanceMapCb = computeDistanceMapFromTertiaryStructure(context, tertiaryStructure, true);
    return distanceMapCb;
  }
  else
  {
    if (!distanceMapCa && tertiaryStructure && tertiaryStructure->hasCAlphaAtoms())
      const_cast<Protein* >(this)->distanceMapCa = computeDistanceMapFromTertiaryStructure(context, tertiaryStructure, false);
    return distanceMapCa;
  }
}

SymmetricMatrixPtr Protein::getContactMap(ExecutionContext& context, double threshold, bool betweenCBetaAtoms) const
{
  jassert(threshold == 8.0);
  SymmetricMatrixPtr* res = const_cast<SymmetricMatrixPtr* >(betweenCBetaAtoms ? &contactMap8Cb : &contactMap8Ca);
  if (!*res)
  {
    SymmetricMatrixPtr distanceMap = getDistanceMap(context, betweenCBetaAtoms);
    if (distanceMap)
      *res = computeContactMapFromDistanceMap(distanceMapCa, threshold);
  }
  return *res;
}

const SymmetricMatrixPtr& Protein::getDisulfideBonds(ExecutionContext& context) const
{
  if (!disulfideBonds && tertiaryStructure)
  {
    SymmetricMatrixPtr distanceMap = tertiaryStructure->makeSulfurDistanceMatrix(context, cysteinIndices);
    if (distanceMap)
      const_cast<Protein* >(this)->disulfideBonds = computeDisulfideBondsFromTertiaryStructure(distanceMap);
  }
  return disulfideBonds;
}

const MatrixPtr& Protein::getFullDisulfideBonds(ExecutionContext& context) const
{
  if (fullDisulfideBonds)
    return fullDisulfideBonds;

  const SymmetricMatrixPtr& symmetricBonds = getDisulfideBonds(context);
  if (symmetricBonds)
    const_cast<Protein* >(this)->fullDisulfideBonds = symmetricBonds->toMatrix();

  return fullDisulfideBonds;
}

const DoubleVectorPtr& Protein::getCysteinBondingStates(ExecutionContext& context) const
{
  const double threshold = 0.5;
  if (cysteinBondingStates)
    return cysteinBondingStates;
  const SymmetricMatrixPtr& disulfideMap = getDisulfideBonds(context);
  if (!disulfideMap)
    return cysteinBondingStates;

  const size_t n = disulfideMap->getDimension();
  const_cast<Protein* >(this)->cysteinBondingStates = Protein::createEmptyProbabilitySequence(n);
  for (size_t i = 0; i < n; ++i)
    cysteinBondingStates->setElement(i, probability(0.0));

  if (n < 2)
    return cysteinBondingStates;
  
  for (size_t i = 0; i < n - 1; ++i)
    for (size_t j = i + 1; j < n; ++j)
    {
      const double value = disulfideMap->getElement(i, j).getDouble();
      if (value > threshold)
      {
        cysteinBondingStates->setElement(i, probability(value));
        cysteinBondingStates->setElement(j, probability(value));
      }
    }

  return cysteinBondingStates;
}

size_t Protein::getNumBondedCysteins() const
{
  if (!cysteinBondingStates)
    return 0;

  const size_t numCysteins = cysteinBondingStates->getNumElements();
  size_t numBondedCysteins = 0;
  for (size_t i = 0; i < numCysteins; ++i)
    if (cysteinBondingStates->getElement(i).getDouble() > 0.5)
      ++numBondedCysteins;

  return numBondedCysteins;
}

CartesianPositionVectorPtr Protein::getCAlphaTrace() const
{
  if (!calphaTrace && tertiaryStructure)
    const_cast<Protein* >(this)->calphaTrace = computeCAlphaTraceFromTertiaryStructure(tertiaryStructure);
  return calphaTrace;
}

/*
** Converters
*/
DoubleVectorPtr Protein::computeCysteinBondingProperty(DoubleVectorPtr bondingStates)
{
  if (!bondingStates)
    return DoubleVectorPtr();

  const size_t numCysteins = bondingStates->getNumElements();
  size_t numBondedCysteins = 0;
  for (size_t i = 0; i < numCysteins; ++i)
    if (bondingStates->getElement(i).getDouble() > 0.5)
      ++numBondedCysteins;

  SparseDoubleVectorPtr res = new SparseDoubleVector(sparseDoubleVectorClass(cysteinBondingPropertyElementEnumeration, probabilityType));

  if (!numBondedCysteins)
    res->setElement(none, probability(1.0));
  else if (numBondedCysteins == numCysteins)
    res->setElement(all, probability(1.0));
  else
    res->setElement(mix, probability(1.0));

  return res;
}

DoubleVectorPtr Protein::computeDisorderRegionsFromTertiaryStructure(TertiaryStructurePtr tertiaryStructure)
{
  if (!tertiaryStructure)
  {
    jassert(false);
    return DoubleVectorPtr();
  }

  size_t n = tertiaryStructure->getNumResidues();
  DenseDoubleVectorPtr res = createEmptyProbabilitySequence(n);
  for (size_t i = 0; i < n; ++i)
    res->getValueReference(i) = tertiaryStructure->getResidue(i) == ResiduePtr() ? 1.0 : 0.0;
  
  static const int minimumDisorderLength = 4;
  for (size_t i = 0; i < n; )
  {
    if (res->getValueReference(i) == 1.0)
    {
      size_t j = i + 1;
      while (j < n && res->getValueReference(j) == 1.0) ++j;
      if ((j - i) < (size_t)minimumDisorderLength)
        for (size_t ii = i; ii < j; ++ii)
          res->getValueReference(ii) = 0.0;
      i = j;
    }
    else
      ++i;
  }
  
  return res;
}

ContainerPtr Protein::computeSecondaryStructureFromDSSPSecondaryStructure(ContainerPtr dsspSecondaryStructure)
{
  size_t n = dsspSecondaryStructure->getNumElements();
  ClassPtr ss8SparseVectorClass = sparseDoubleVectorClass(secondaryStructureElementEnumeration, probabilityType);
  ContainerPtr res = objectVector(ss8SparseVectorClass, n);
  for (size_t i = 0; i < n; ++i)
  {
    DoubleVectorPtr dssp = dsspSecondaryStructure->getElement(i).getObjectAndCast<DoubleVector>();
    if (dssp)
    {
      SparseDoubleVectorPtr value = new SparseDoubleVector(ss8SparseVectorClass);
      value->appendValue(dsspSecondaryStructureToSecondaryStructure((DSSPSecondaryStructureElement)dssp->getIndexOfMaximumValue()), 1.0);
      res->setElement(i, value);
    }
  }
  return res;
}

DoubleVectorPtr Protein::computeBinarySolventAccessibilityFromSolventAccessibility(DoubleVectorPtr solventAccessibility, double threshold)
{
  size_t n = solventAccessibility->getNumElements();
  DenseDoubleVectorPtr res = createEmptyProbabilitySequence(n);
  for (size_t i = 0; i < n; ++i)
  {
    Variable sa = solventAccessibility->getElement(i);
    if (sa.exists())
      res->getValueReference(i) = sa.getDouble() > threshold ? 1.0 : 0.0;
  }
  return res;
}

SymmetricMatrixPtr Protein::computeContactMapFromDistanceMap(SymmetricMatrixPtr distanceMap, double threshold)
{
  size_t n = distanceMap->getDimension();
  SymmetricMatrixPtr res = Protein::createEmptyContactMap(distanceMap->getDimension());
  for (size_t i = 0; i < n; ++i)
    for (size_t j = i; j < n; ++j)
    {
      Variable distance = distanceMap->getElement(i, j);
      if (distance.exists())
      {
        jassert(distance.isDouble());
        res->setElement(i, j, Variable(distance.getDouble() <= threshold ? 1.0 : 0.0, probabilityType));
      }
    }
  return res;
}

void computeDisulfideBondsFromTertiaryStructure(const SymmetricMatrixPtr& distanceMap, std::vector<bool>& isBridged, double distanceThreshold, SymmetricMatrixPtr& result)
{
  size_t n = distanceMap->getDimension();
  for (size_t i = 0; i < n - 1; ++i)
  {
    if (isBridged[i])
      continue;

    double bestDistance = DBL_MAX;
    size_t bestIndex = n;
    for (size_t j = i + 1; j < n; ++j)
    {
      double value = distanceMap->getElement(i, j).getDouble();
      if (value < bestDistance)
      {
        bestDistance = value;
        bestIndex = j;
      }
    }
    jassert(bestIndex < n);
    
    if (isBridged[bestIndex])
      continue;
    
    if (bestDistance < distanceThreshold)
    {
      result->setElement(i, bestIndex, probability(1.0));
      isBridged[i] = true;
      isBridged[bestIndex] = true;
    }
  }
}


SymmetricMatrixPtr Protein::computeDisulfideBondsFromTertiaryStructure(SymmetricMatrixPtr distanceMap)
{
  static const double disulfideBondSulfurDistanceStrongThreshold = 2.25;
  static const double disulfideBondSulfurDistanceSoftThreshold = 3.0;
  jassert(distanceMap);
  size_t n = distanceMap->getDimension();
  SymmetricMatrixPtr res = Protein::createEmptyContactMap(n);

  if (n == 0)
    return res;

  std::vector<bool> isBridged(n, false);
  /* Initialize connectivity to 0 */
  for (size_t i = 0; i < n; ++i)
    for (size_t j = i; j < n; ++j)
      res->setElement(i, j, probability(0.0));

  /* Fill matrix */
  ::computeDisulfideBondsFromTertiaryStructure(distanceMap, isBridged, disulfideBondSulfurDistanceStrongThreshold, res);
  ::computeDisulfideBondsFromTertiaryStructure(distanceMap, isBridged, disulfideBondSulfurDistanceSoftThreshold, res);

  return res;
}

ContainerPtr Protein::computeStructuralAlphabetSequenceFromCAlphaTrace(CartesianPositionVectorPtr calphaTrace)
{
  /*
  ** Descriptors from "A HMM derived structural alphabet for proteins" (2004)
  */
  static const double structuralAlphabetDescriptiors[27][4] = {
    {5.39 ,5.09 ,5.38 ,2.92},
    {5.43 ,5.09 ,5.42 ,2.94},
    {5.41 ,5.23 ,5.61 ,2.86},
    {5.62 ,5.25 ,5.42 ,2.87},
    {5.59 ,5.49 ,5.78 ,2.58},
    {5.40 ,5.58 ,5.42 ,3.39},
    {5.78 ,5.68 ,6.07 ,1.46},
    {5.55 ,7.74 ,5.60 ,-3.31},
    {5.60 ,6.71 ,5.50 ,3.69},
    {5.69 ,8.09 ,5.67 ,3.09},
    {5.66 ,8.95 ,6.54 ,2.09},
    {5.66 ,8.91 ,6.66 ,-1.46},
    {5.66 ,8.07 ,6.70 ,2.96},
    {5.70 ,7.26 ,7.02 ,0.88},
    {6.03 ,6.85 ,5.64 ,-0.63},
    {6.47 ,5.92 ,5.56 ,0.53},
    {6.57 ,8.96 ,5.58 ,-2.19},
    {6.71 ,8.27 ,5.47 ,-3.56},
    {6.21 ,9.21 ,5.77 ,0.27},
    {6.87 ,8.28 ,6.03 ,-3.44},
    {6.89 ,8.94 ,6.76 ,-0.48},
    {6.72 ,9.12 ,6.41 ,-3.31},
    {6.71 ,9.64 ,6.50 ,-2.60},
    {6.39 ,9.93 ,6.75 ,-1.07},
    {6.87 ,10.06 ,6.51 ,-1.41},
    {6.48 ,10.17 ,7.09 ,0.66},
    {6.80 ,10.35 ,6.85 ,-0.25}
  };

  size_t n = calphaTrace->getNumElements();
  ContainerPtr res = objectVector(sparseDoubleVectorClass(structuralAlphabetElementEnumeration, probabilityType), n);
  for (size_t i = 3; i < n; ++i)
  {
    impl::Vector3 a = calphaTrace->getPosition(i - 3);
    impl::Vector3 b = calphaTrace->getPosition(i - 2);
    impl::Vector3 c = calphaTrace->getPosition(i - 1);
    impl::Vector3 d = calphaTrace->getPosition(i);
    if (!a.exists() || !b.exists() || !c.exists() || !d.exists())
      continue;
    
    double d1 = (a - c).l2norm();
    double d2 = (a - d).l2norm();
    double d3 = (b - d).l2norm();
    
    impl::Vector3 normalVector =  (b - a).crossProduct(c - a);
    
    double d4 = (normalVector.dotProduct(d) - normalVector.dotProduct(a)) / normalVector.l2norm();
    
    int bestGroup = 0;
    double minDistance = DBL_MAX;
    for (size_t j = 0; j < 27; ++j)
    {
      double diff_1 = d1 - structuralAlphabetDescriptiors[j][0];
      double diff_2 = d2 - structuralAlphabetDescriptiors[j][1];
      double diff_3 = d3 - structuralAlphabetDescriptiors[j][2];
      double diff_4 = d4 - structuralAlphabetDescriptiors[j][3];
      
      diff_1 *= diff_1;
      diff_2 *= diff_2;
      diff_3 *= diff_3;
      diff_4 *= diff_4;
      
      double dist = sqrt(diff_1 + diff_2 + diff_3 + diff_4);
      
      if (dist < minDistance)
      {
        minDistance = dist;
        bestGroup = (int)j;
      }
    }
    SparseDoubleVectorPtr value = new SparseDoubleVector(structuralAlphabetElementEnumeration, probabilityType);
    value->appendValue(bestGroup, 1.0); 
    res->setElement(i - 2, value);
  }
  return res;
}

/*
** Create Empty Targets
*/
ContainerPtr Protein::createEmptyPositionSpecificScoringMatrix(size_t length)
  {return objectVector(denseDoubleVectorClass(positionSpecificScoringMatrixEnumeration, probabilityType), length);}

ContainerPtr Protein::createEmptyDSSPSecondaryStructure(size_t length, bool useSparseVectors)
{
  TypePtr elementsType = useSparseVectors
    ? sparseDoubleVectorClass(dsspSecondaryStructureElementEnumeration, probabilityType)
    : denseDoubleVectorClass(dsspSecondaryStructureElementEnumeration, probabilityType);
  return objectVector(elementsType, length);
}

DoubleVectorPtr Protein::createEmptyProbabilitySequence(size_t length)
  {return new DenseDoubleVector(positiveIntegerEnumerationEnumeration, probabilityType, length);}

DoubleVectorPtr Protein::createEmptyDoubleSequence(size_t length)
  {return new DenseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType, length);}

SymmetricMatrixPtr Protein::createEmptyContactMap(size_t length)
  {return new DoubleSymmetricMatrix(probabilityType, length, Variable::missingValue(probabilityType).getDouble());}

SymmetricMatrixPtr Protein::createEmptyDistanceMap(size_t length)
  {return new DoubleSymmetricMatrix(angstromDistanceType, length, Variable::missingValue(angstromDistanceType).getDouble());}

/*
** Lua
*/
int Protein::fromFile(LuaState& state)
{
  ExecutionContext& context = state.getContext();
  File file = state.checkFile(1);
  String ext = file.getFileExtension();
  ProteinPtr res;
  if (ext == T(".pdb"))
    res = Protein::createFromPDB(context, file);
  else if (ext == T(".fasta"))
    res = Protein::createFromFASTA(context, file);
  else
    res = Protein::createFromXml(context, file);
  return state.returnObject(res);
}

int Protein::fromDirectory(LuaState& state)
{
  ExecutionContext& context = state.getContext();
  File directory = state.checkFile(1);
  int maxCount = 0;
  String scopeName = T("Loading Proteins");
  if (state.getTop() >= 2)
    maxCount = state.checkInteger(2);
  if (state.getTop() >= 3)
    scopeName = state.checkString(3);
  return state.returnObject(Protein::loadProteinsFromDirectory(context, directory, maxCount, scopeName));
}

int Protein::length(LuaState& state)
{
  ProteinPtr protein = state.checkObject(1, proteinClass).staticCast<Protein>();
  state.pushInteger(protein->getLength());
  return 1;
}
