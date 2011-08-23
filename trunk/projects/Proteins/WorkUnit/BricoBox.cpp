#include "precompiled.h"
#include "BricoBox.h"

using namespace lbcpp;

size_t CheckDisulfideBondsWorkUnit::getNumBridges(ExecutionContext& context, ProteinPtr protein)
{
  SymmetricMatrixPtr bridges = protein->getDisulfideBonds(context);
  
  if (!checkConsistencyOfBridges(bridges))
  {
    jassertfalse;
    return 0;
  }
  
  size_t n = bridges->getDimension();
  double numBridges = 0;
  for (size_t i = 0; i < n; ++i)
    for (size_t j = i + 1; j < n; ++j)
      numBridges += (size_t)bridges->getElement(i, j).getDouble();
  return (size_t)numBridges;
}

bool CheckDisulfideBondsWorkUnit::checkConsistencyOfBridges(SymmetricMatrixPtr bridges)
{
  size_t n = bridges->getDimension();
  for (size_t i = 0; i < n; ++i)
  {
    size_t numBridges = 0;
    for (size_t j = 0; j < n; ++j)
    {
      double value = bridges->getElement(i,j).getDouble();
      if (value != 0.0 && value != 1.0)
      {
        jassertfalse;
        return false;
      }
      numBridges += (size_t)value;
    }
    
    if (numBridges > 1)
    {
      jassertfalse;
      return false;
    }
  }
  return true;
}
