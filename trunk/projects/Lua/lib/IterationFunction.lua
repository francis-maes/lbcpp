-- Francis Maes, 01/08/2011
-- Iteration Functions

--[[
Interface:

 IterationFunction.one
 IterationFunction.constant(value)
 IterationFunction.invLinear(initialValue, halfPeriod)

]]

module("IterationFunction", package.seeall)

function one(epoch)
  return 1.0
end

function constant(value)
  return function (epoch) return value end
end

function invLinear(initialValue, halfPeriod)
  return function (epoch) return initialValue * halfPeriod / (halfPeriod + epoch) end
end