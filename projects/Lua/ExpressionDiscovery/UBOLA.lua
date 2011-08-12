-- Francis Maes, 11/08/2011
-- Upper Bound Open Loop Action Search

require '../ExpressionDiscovery/DecisionProblem'
require 'Predictor'

subspecified function DecisionProblem.Ubola(problem, x0)

  parameter numEpisodes = {default = 10000}
  parameter numEpisodesPerIteration = {default = 100}
  parameter C = {default = 1}
  parameter alpha = {default = 0.001}
  parameter verbose = {default = false}

  local episodeNumber = 1
  x0 = x0 or problem.x0

  ---------------------

  local predictor = Predictor.ConditionalGaussian{thetaMu = Vector.newDense(), thetaSigma = Vector.newDense()}

  local lossStats
  local function addExample(features, score)
    local loss, dlossdmu, dlossdsigma = predictor.lossAndGradient(features, score)
    lossStats:observe(loss)
    --print ("loss: ", loss, dlossdmu:l2norm(), dlossdsigma:l2norm())
    predictor.__parameters.thetaMu:add(dlossdmu, -alpha)
    predictor.__parameters.thetaSigma:add(dlossdsigma, -alpha)
  end

  ---------------------

  local function playEpisode()
    local x = x0
    local score = 0
    local actionSequence = {}
    local featuresSequence = {}
    while not problem.isFinal(x) do
      local U = problem.U(x)
      local bestScore = -math.huge
      local bestFeatures
      local bestAction
      for i,u in ipairs(U) do
        local features = problem:actionFeatures(x, u)
        local mu, sigma = predictor(features)
        local score = mu + C * sigma
        if score > bestScore then
          bestScore = score
          bestFeatures = features
          bestAction = u
        end
      end
      score = score + problem.g(x, bestAction)
      x = problem.f(x, bestAction)
      table.insert(actionSequence, bestAction)
      table.insert(featuresSequence, bestFeatures)
    end
    return x, actionSequence, score, featuresSequence
  end

  local function backPropagate(featuresSequence, score)
    for t,features in ipairs(featuresSequence) do
      addExample(features, score)
    end
  end

  local bestScore = -math.huge
  local bestActionSequence
  local bestFinalState

  local function episode()
    local finalState, actionSequence, score, featuresSequence = playEpisode()
    if verbose then
      context:information(problem.stateToString(finalState), "->", score)
    end
    
    if score > bestScore then
      bestFinalState = finalState
      bestActionSequence = actionSequence
      bestScore = score
    end

    backPropagate(featuresSequence, score)
  end

  local numIterations = math.ceil(numEpisodes / numEpisodesPerIteration)
  for iter=1,numIterations do
    context:enter("Episodes " .. episodeNumber .. " -- " .. (episodeNumber + numEpisodesPerIteration - 1))
    lossStats = Statistics:mean()
    for e=1,numEpisodesPerIteration do
      episode()
      episodeNumber = episodeNumber + 1
    end
    context:result("iteration", iter)
    context:result("mean loss", lossStats:getMean())
    context:result("best score", bestScore)
    context:result("best final state", problem.stateToString(bestFinalState))
    context:leave()
  end

  context:result("theta mu", predictor.__parameters.thetaMu)
  context:result("theta sigma", predictor.__parameters.thetaSigma)

  return bestFinalState, bestActionSequence, bestScore
end