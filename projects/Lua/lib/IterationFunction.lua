-- Francis Maes, 01/08/2011
-- Iteration Functions

--[[
Interface:

 IterationFunction.one
 IterationFunction.constant(value)
 IterationFunction.invLinear(initialValue, halfPeriod)

]]

module("IterationFunction", package.seeall)

one = |epoch| 1

subspecified function constant(epoch)
  parameter value = {default = 1}
  return value
end

subspecified function invLinear(epoch)
  parameter initialValue = {default = 2}
  parameter halfPeriod = {default = 1000}
  return initialValue * halfPeriod / (halfPeriod + epoch)
end
