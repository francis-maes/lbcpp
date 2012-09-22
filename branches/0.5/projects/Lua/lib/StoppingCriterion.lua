-- Francis Maes, 11/08/2011
-- Stopping criterions

--[[
Interface:

]]

StoppingCriterion = {}

subspecified function StoppingCriterion.MaxIterations(iteration, objectiveValue)
  parameter maxIterations = {default = 100}
  return iteration >= maxIterations
end

StoppingCriterion.MaxIterationsWithoutImprovement = subspecified {
  parameter maxIterationsWithoutImprovement = {default = 5},
  numIterationsWithoutImprovement = 0,
  bestValue = math.huge,

  shouldStop = function (self, iteration, objectiveValue)
    local epsilon = 1e-10
    if objectiveValue < self.bestValue - epsilon then -- reject too small improvements
      self.bestValue = objectiveValueToMinimize;
      self.numIterationsWithoutImprovement = 0;
    else
      self.numIterationsWithoutImprovement = self.numIterationsWithoutImprovement + 1
      if self.numIterationsWithoutImprovement > maxIterationsWithoutImprovement then
        return true
      end
    end
    return false
  end,

  reset = function (self)
    self.numIterationsWithoutImprovement = 0
    self.bestValue = math.huge
  end
}

return StoppingCriterion