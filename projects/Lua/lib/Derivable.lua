-- Francis Maes, 02/08/2011
-- Derivable extension

--[[
Interface:

  MT                              metatable for derivable functions
  callIfExists(f, defValue, ...)  if f then f(...) else defValue

]]

module("Derivable", package.seeall)

MT = {  -- metatable
   __call = function (tbl, ...) return tbl.f(...) end,
   
   __index = function (tbl, key)

      if type(key) == "number" then
        local p = tbl.prototype[key] 
        return p ~= nil and tbl[p] or nil
      else
        return tbl
      end
  end
}

function callIfExists(f, defValue, ...)
  if f == nil then
    return defValue
  else
    return f(...)
  end
end

function ternaryOperator(cond, exprIfTrue, exprIfFalse)
  return cond and exprIfTrue or exprIfFalse
end
