-- Francis Maes, 01/08/2011
-- Sparse, Dense and Composed Vectors

--[[
Interface:

 Vector.newSparse()           creates a new sparse vector
 Vector.newDense()            creates a new dense vector
 
 Vector.add(v1, v2, weight)   v1 += v2 * weight, weight is optional
 Vector.dot(v1, v2)           returns the dot product between two vectors
 Vector.l2norm(v)             returns the l2-norm of a vector
]]

Vector = {}

function Vector.newSparse()
  return lbcpp.Object.create("SparseDoubleVector<PositiveIntegerEnumeration, Double>")
end

function Vector.newDense()
  return lbcpp.Object.create("DenseDoubleVector<PositiveIntegerEnumeration, Double>")
end

return Vector
