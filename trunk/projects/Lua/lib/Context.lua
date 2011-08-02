-- Francis Maes, 01/08/2011
-- Sparse, Dense and Composed Vectors

--[[
Interface:

 Context:error(what, where = "")
 Context:warning(what, where = "")
 Context:information(what, where = "")
 Context:result(string, value)
 Context:progress(value, total = 1.0, unit = "")

 Context:enter(description)
 Context:leave(returnValue)

 Context:call(description, function, arguments)  -- self:enter(description) res = function(arguments) self:leave(res)
]]

module("Context", package.seeall)

function call(self, description, fun, ...)
  context:enter(description)
  local res = fun(...)
  context:leave(res)
  return res
end
