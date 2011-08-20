-- Francis Maes, 03/08/2011
-- Random generator

--[[
Interface:
   
   Random.new(seed)   create a new Random Generator
   
   random:sample()    sample a double uniformly in [0,1[
   
]]

Random = {}

function Random.new(seed)
  return lbcpp.RandomGenerator.create(seed)
end

return Random