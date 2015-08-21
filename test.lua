local ffi = require('ffi')
ffi.cdef((module:load("values.h"):gsub("#[^\n]*", "")))
local uscript = module:action("values.so", ffi.load)

local function pop(t) return table.remove(t) end
ffi.metatype("value_t", {
  __tostring = function (cdata)
    return cdata.type == uscript.Tuple and "<tuple len=" .. cdata.tuple.len .. " ref=" .. cdata.tuple.ref .. ">"
        or cdata.type == uscript.Buffer and "<buffer len=" .. cdata.buffer.len .. " ref=" .. cdata.buffer.ref .. ">"
        or cdata.type == uscript.Integer and "<integer " .. tostring(cdata.num) .. ">"
        or "<bool " .. (cdata.num ~= 1 and "true" or "false") .. ">"
  end,
  __len = function (cdata)
    return cdata.type == uscript.Tuple and cdata.tuple.len
        or cdata.type == uscript.Buffer and cdata.buffer.len
  end,
  __index = function (cdata, i)
    return cdata.type == uscript.Tuple and cdata.tuple.values[i - 1]
        or cdata.type == uscript.Buffer and cdata.buffer.bytes[i - 1]
  end,
  __gc = function (cdata)
    p("GC!", cdata)
  end
})
ffi.metatype("buffer_t", {
  __tostring = function (cdata)
    return ffi.string(cdata.bytes, cdata.len)
  end,
})

p(uscript)

do
  local t = uscript.new_tuple(4)
  local b = uscript.new_bool(true)
  t.tuple.values[0] = b
  p(b)
  b = uscript.new_integer(42)
  t.tuple.values[1] = b
  p(b)
  b = uscript.new_buffer(10)
  t.tuple.values[2] = b
  p(b)
  b.buffer.bytes[0] = string.byte('h', 1)
  b.buffer.bytes[2] = string.byte('e', 1)
  b.buffer.bytes[4] = string.byte('l', 1)
  p(tostring(b.buffer))
  b = uscript.new_string("Hello World")
  t.tuple.values[3] = b
  p(b)
  p(tostring(b.buffer))

  p(t, #t)
  for i = 1, #t do
    p(i, t[i])
  end

  local t2 = uscript.clone(t)
  p(t, t2)

end
print(collectgarbage("count")*1024)
collectgarbage()
print(collectgarbage("count")*1024)
collectgarbage()
print(collectgarbage("count")*1024)
