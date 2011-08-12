-- Francis Maes, 11/08/2011
-- Upper Bound Open Loop Action Search

require 'DecisionProblem'

-- todo ...

subspecified function DecisionProblem.Ubola(x0, f)

  parameter numEpisodes = {default = 10000}
  parameter numEpisodesPerIteration = {default = 100}

  local episodeNumber = 1

  local function episode()

    

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

