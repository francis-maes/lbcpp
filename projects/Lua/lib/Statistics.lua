-- Francis Maes, 01/08/2011
-- Incremental Statistics Counters

--[[
Interface:
 
  Statistics.mean()
  Statistics.meanAndVariance()
  Statistics.meanVarianceAndBounds()
  
  Statistics:observe(value, weight = 1.0)
  Statistics:clear()
  
  Statistics:getMean()              -- sum of values / sum of weights
  Statistics:getStandardDeviation() -- standard deviation
  Statistics:getVariance()          -- variance
  Statistics:getMinimum()           -- minimum observed value
  Statistics:getMaximum()           -- maximum observed value
  Statistics:getSum()               -- sum of values
  Statistics:getCount()             -- sum of weights
  
]]

module("Statistics", package.seeall)

function mean() 
  return lbcpp.Object.create("ScalarVariableMean")
end

function meanAndVariance()
  return lbcpp.Object.create("ScalarVariableMeanAndVariance")
end

function meanVarianceAndBounds()
  return lbcpp.Object.create("ScalarVariableStatistics")
end
