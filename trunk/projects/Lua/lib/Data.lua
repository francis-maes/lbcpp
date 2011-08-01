-- Francis Maes, 01/08/2011
-- Data container

module("Data", package.seeall)

function load(stream, maxCount, ...)
  local res = {}
  local co = coroutine.create(stream)
  repeat
    local errorfree, element = coroutine.resume(co, ...)
    if not errorfree then
      print("error")
      return
    end
    table.insert(res, element)
    if maxCount > 0 and #res >= maxCount then
      break
    end
  until coroutine.status(co) == 'dead'
  return res
end  
