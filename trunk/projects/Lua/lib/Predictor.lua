-- Francis Maes, 11/08/2011
-- Predictors

--[[
Interface:
  Predictor.LinearBinaryClassifier
]]

require 'Loss'

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


return Predictor