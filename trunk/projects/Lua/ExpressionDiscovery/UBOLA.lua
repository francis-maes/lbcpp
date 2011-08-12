-- Francis Maes, 11/08/2011
-- Upper Bound Open Loop Action Search

require '../ExpressionDiscovery/DecisionProblem'
require 'Predictor'

-- todo ...

--local function gaussianDensity(derivable mu, derivable sigma, y)
--  local ny = (y - mu) / sigma
--  return math.exp(-ny^2 / 2) / (math.sqrt(2 * math.pi) * sigma)
--end
--local function gaussianNLL(derivable mu, derivable sigma, y)
--  return -math.log(gaussianDensity(mu, sigma, y))
--end

local function gaussianNLL(derivable mu, derivable sigma, y)
  local ny = (y - mu) / sigma
  return math.log(math.sqrt(2 * math.pi) * sigma) + 0.5 * ny ^ 2
end

local function condGaussianNLL(derivable mu, derivable sigmaParam, y)
  local sigma = math.log(1 + math.exp(-sigmaParam))
  return gaussianNLL(mu, sigma, y)
end


Predictor.ConditionalGaussian = subspecified setmetatable({
  parameter thetaMu = {default = Vector.newDense()},
  parameter thetaSigma = {default = Vector.newDense()},

  predict = function (x)
    local mu = thetaMu:dot(x)
    local sigma = math.log(1 + math.exp(-thetaSigma:dot(x)))
    return mu, sigma
  end,

  lossAndGradient = function (x, supervision)
    local mu = thetaMu:dot(x)
    local sigmaParam = thetaSigma:dot(x)
    local loss = condGaussianNLL(mu, sigmaParam, supervision)
    local dlossdmu = condGaussianNLL[1](mu, sigmaParam, supervision)
    local dlossdsigmaParam = condGaussianNLL[2](mu, sigmaParam, supervision)
    return loss, x * dlossdmu, x * dlossdsigmaParam

  end
}, Predictor.MT)



subspecified function DecisionProblem.Ubola(problem, x0)

  parameter numEpisodes = {default = 10000}
  parameter numEpisodesPerIteration = {default = 100}
  parameter C = {default = 1}
  parameter verbose = {default = false}

  local episodeNumber = 1

  ---------------------

  local predictor = Predictor.ConditionalGaussian{}

  local function addExample(features, score)
    local loss, dlossdmu, dlossdsigma = predictor.lossAndGradient(features, score)
    print ("loss: ", loss, dlossdmu:l2norm(), dlossdsigma:l2norm())
    predictor.__parameters.thetaMu:add(dlossdmu, -0.01)
    predictor.__parameters.thetaSigma:add(dlossdsigma, -0.01)
  end

  ---------------------

  local function heuristic(problem, x, u)
    local mu, sigma = predictor(problem.actionFeatures(x, u))    
    return mu + C * sigma
  end

  local function playEpisode()
    return DecisionProblem.doEpisode(problem, x, DecisionProblem.ActionValueBasedPolicy{actionValues = heuristic})
  end

  local function backPropagate(actionSequence, score)
    local x = x0 or problem.x0
    for t,u in ipairs(actionSequence) do
      addExample(problem.actionFeatures(x, u), score)
      x = problem.f(x, u)
    end
    assert(problem.isFinal(x))
  end

  local function episode()
    local finalState, actionSequence, score = playEpisode()
    if verbose then
      context:information(problem.stateToString(finalState), "->", score)
    end
    backPropagate(actionSequence, score)
  end

  local numIterations = math.ceil(numEpisodes / numEpisodesPerIteration)
  for iter=1,numIterations do
    context:enter("Episodes " .. episodeNumber .. " -- " .. (episodeNumber + numEpisodesPerIteration - 1))
    for e=1,numEpisodesPerIteration do
      episode()
      episodeNumber = episodeNumber + 1
    end
    context:leave()
  end
end