-- Francis Maes, 11/08/2011
-- Predictors

--[[
Interface:
  Predictor.LinearBinaryClassifier
]]

require 'Loss'
require 'Vector'

Predictor = {}
Predictor.MT = {} -- Metatable
 
function Predictor.MT.__call(tbl, ...)
  return tbl.predict(...)
end

Predictor.LinearBinaryClassifier = subspecified setmetatable({
  parameter loss = {default = Loss.binary{Loss.hinge}},
  parameter theta = {default = Vector.newDense()},
  predict = function (x)
    return theta:dot(x) > 0 and 2 or 1
  end,
  lossAndGradient = function (x,supervision)
    local prediction = theta:dot(x)
    --print (prediction,supervision)
    return loss(prediction, supervision), x * loss[1](prediction, supervision)
  end
}, Predictor.MT)



--
-- Conditional Gaussian
-- 

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

return Predictor
