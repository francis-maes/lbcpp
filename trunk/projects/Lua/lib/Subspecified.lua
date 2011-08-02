-- Francis Maes, 02/08/2011
-- Subspecified extension

--[[
Interface:

  Subspecified.instantiate(parameters, functor, paramValues)

]]

module("Subspecified", package.seeall)

instanceMT = {} -- instance metatable
MT = {}         -- class metatable
  
function instanceMT.__call(self, ...)
  return self.get()(...)
end

function MT.__call(expression, paramValues)

 -- create instance
 local res = setmetatable({expression = expression}, instanceMT)

 -- fill parameters
 res.parameters = {}
 for name,properties in pairs(expression.parameters) do
   table.insert(res.parameters, paramValues[name] or properties.default)
 end
 
 -- add the "get()" function
 function res:get()
   return self.expression.functor(self.parameters)
 end

 return res

end
