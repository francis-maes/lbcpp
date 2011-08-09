-- Francis Maes, 01/08/2011
-- Iteration Functions

--[[
Interface:

 IterationFunction.one
 IterationFunction.constant(value)
 IterationFunction.invLinear(initialValue, halfPeriod)

]]

IterationFunction = {}

IterationFunction.one = |epoch| 1

subspecified function IterationFunction.constant(epoch)
  parameter value = {default = 1}
  return value
end

subspecified function IterationFunction.invLinear(epoch)
  parameter initialValue = {default = 2}
  parameter halfPeriod = {default = 1000}
  return initialValue * halfPeriod / (halfPeriod + epoch)
end

return IterationFunction
