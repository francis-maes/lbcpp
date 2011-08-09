require 'Statistics'
require 'Sampler'
require 'Vector'

subspecified function SimpleEDA(objective, sampler)
  
  parameter numIterations = {default=100}
  parameter numCandidates = {default=100}
  parameter numBests = {default=10}

  local bestEverScore = math.huge
  local bestEverCandidate = nil  

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

  for i=1,numIterations do
    context:call("Iteration " .. i, iteration, i)
  end

  return bestEverCandidate, bestEverScore
end

objective = |input| input:l1norm()

eda = SimpleEDA{numIterations=10, numCandidates=100, numBests=10}

Sampler.IndependentVector = subspecified Stochastic.new(
{
  parameter samplers = {},

  sample = function ()
    local res = Vector.newDense(#samplers)
    for i,sampler in ipairs(samplers) do
      res:set(i-1, sampler())
    end
    return res
  end,

  learn = function (samples)
    local n = #samples
    for i,sampler in ipairs(samplers) do
      local subSamples = {}  -- FIXME: use DenseDoubleVector
      for j=1,n do
        subSamples[j] = samples[j]:get(i-1)
      end
      sampler.learn(subSamples)
    end
  end
})


local samplers = {}
for i=1,10 do
  samplers[i] = Sampler.Gaussian{}
end
eda(objective, Sampler.IndependentVector{samplers = samplers})






