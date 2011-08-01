-- Francis Maes, 01/08/2011
-- Sparse, Dense and Composed Vectors

module("Vector", package.seeall)


function newSparse()
  return lbcpp.Object.create("SparseDoubleVector<PositiveIntegerEnumeration, Double>")
end