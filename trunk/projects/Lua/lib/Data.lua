-- Francis Maes, 01/08/2011
-- Data container

Data = {}

function Data.load(stream, maxCount, ...)
  local res = {}
  local co = coroutine.create(stream)
  repeat
    local ok, element = coroutine.resume(co, ...)
    if not ok then
      __errorHandler(element)
      return res
    end
    table.insert(res, element)
    if maxCount > 0 and #res >= maxCount then
      break
    end
  until coroutine.status(co) == 'dead'
  return res
end  

return Data