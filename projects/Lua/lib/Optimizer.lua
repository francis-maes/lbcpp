-- Francis Maes, 11/08/2011
-- Optimizers

require 'Vector'
require 'Statistics'
require 'IterationFunction'
require 'StoppingCriterion'
require 'Stochastic'

Optimizer = {}


function Optimizer.CMAES(params)
  local res = lbcpp.Object.create("CMAESOptimizer")
  for k,v in pairs(params) do
    res[k] = v
  end
  return res
end

function Optimizer.HOO(params)
  local res = lbcpp.Object.create("HOOOptimizer")
  for k,v in pairs(params) do
    res[k] = v
  end
  return res
end



--problem = {
--  initialSolution,
--  numSamples,
--  objective,  --index, parameters -> value, gradient
--  scores, --{name, parameter->score} map
--  principalScore -- score name
--}

-- objective: index, parameters -> value, gradient
-- validation: parameters -> value
subspecified function Optimizer.StochasticGradientDescent(problem)
  -- todo: parameter randomizeExamples = {default = true}
  parameter rate = {default = IterationFunction.constant{1}}
  parameter stoppingCriterion = {default = nil}
  parameter maxIterations = {default = 0, min = 0}
  parameter restoreBestParameters = {default = false}
  parameter randomizeSamples = {default = true}

  local parameters = problem.initialSolution
  assert(parameters)
  local iter = 1
  local epoch = 1

  local principalScore = -math.huge
  local bestPrincipalScore = -math.huge
  local bestParameters = parameters

  local function iteration()
    local scoreStats = Statistics.mean()
    local order = randomizeSamples and Stochastic.uniformOrder(problem.numSamples)
    for i=1,problem.numSamples do
      local index = order and order[i] or i
      local score, gradient = problem.objective(index, parameters, epoch)
      --print (score,gradient)
      scoreStats:observe(score)
      if gradient then
        parameters:add(gradient, -rate(epoch))
        --print (parameters, parameters:l2norm())
      end
      epoch = epoch + 1
    end

    context:result("iteration", iter)
    context:result("score stats", scoreStats)
    context:result("mean score", scoreStats:getMean())
    context:result("parameters l0norm", parameters:l0norm())
    context:result("parameters l1norm", parameters:l1norm())
    context:result("parameters l2norm", parameters:l2norm())
    context:result("parameters", parameters:clone())
    context:result("rate", rate(epoch))
    context:result("epoch", epoch)

    for scoreName,scoreFunction in pairs(problem.scores) do
      local score = scoreFunction(parameters)
      context:result(scoreName, score)
      if scoreName == problem.principalScore then
        principalScore = score
          print ("principal score", principalScore)
      end
    end
    if restoreBestParameters then
      assert(principalScore)
      if principalScore > bestPrincipalScore then
        bestPrincipalScore = principalScore
        bestParameters = parameters:clone()
      end
    end
  end

  if stoppingCriterion then
    stoppingCriterion:reset()
  end
  while maxIterations == 0 or iter <= maxIterations do
    context:call("Iteration " .. iter,  iteration)
    if stoppingCriterion and stoppingCriterion:shouldStop(iter, validationScore) then
      break
    end
    iter = iter + 1
  end

  if restoreBestParameters then
    parameters = bestParameters
    principalScore = bestPrincipalScore
    print ("Restoring best parameters with score ", principalScore)
  end
  return parameters, principalScore
end

subspecified function Optimizer.EstimationOfDistributionAlgorithm(objective, sampler)
  
  parameter numCandidates = {default=100}
  parameter numBests = {default=10}
  parameter stoppingCriterion = {default = nil}
  parameter maxIterations = {default = 0, min = 0}
  
  local bestEverScore = math.huge
  local bestEverCandidate = nil
  local validationScore = math.huge

  local function iteration(i)

    local population = {}
    local candidates = {}
    local bestIterationScore = math.huge
    local bestIterationCandidate = nil
    
    -- sample candidates and score them
    for c=1,numCandidates do
      local candidate = sampler()
      local score = objective(candidate)
      --print ("candidate: " .. candidate .. " score " .. score)
      if score < bestIterationScore then
        bestIterationScore = score
        bestIterationCandidate = candidate
      end
      table.insert(candidates, {candidate, score})
    end

    validationScore = bestIterationScore
    if bestIterationScore < bestEverScore then
      bestEverScore = bestIterationScore
      bestEverCandidate = bestIterationCandidate
    end

    -- sort scores
    table.sort(candidates, |a,b| a[2] < b[2])

    -- pick best candidates
    local bests = {}
    for c=1,numBests do
      table.insert(bests, candidates[c][1])
    end
    
    -- learn new sampler
    sampler.learn(bests)

    context:result("iteration", i)
    context:result("bestIterationScore", bestIterationScore)
    context:result("bestIterationCandidate", bestIterationCandidate)
    context:result("bestEverScore", bestEverScore)
    context:result("bestEverCandidate", bestEverCandidate)
    
    return bestIterationScore
  end  

  local iter = 1
  while maxIterations == 0 or iter <= maxIterations do
    context:call("Iteration " .. iter,  iteration)
    if stoppingCriterion and stoppingCriterion:shouldStop(iter, validationScore) then
      break
    end
    iter = iter + 1
  end

  return bestEverCandidate, bestEverScore
end

return Optimizer