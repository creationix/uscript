-- Replace with host to mcu
local host = "192.168.1.129"

local connect = require('coro-net').connect
local httpCodec = require('http-codec')
local websocketCodec = require('websocket-codec')
local wrapper = require('coro-wrapper')
local readWrap, writeWrap = wrapper.reader, wrapper.writer
local readLine = require('readline').readLine
local split = require('coro-split')

local options = require('pretty-print')
local function getLine()
  local thread = coroutine.running()
  readLine("> ", options, function (err, data)
    assert(coroutine.resume(thread, data, err))
  end)
  return coroutine.yield()
end

coroutine.wrap(function ()
  print("Connecting to websocket server: " .. host)
  local rawRead, rawWrite = connect({
    host = host,
    port = 80,
  })
  local read, updateDecoder = readWrap(rawRead, httpCodec.decoder())
  local write, updateEncoder = writeWrap(rawWrite, httpCodec.encoder())

  assert(websocketCodec.handshake({
    host = host,
  }, function (req)
    write(req)
    return read()
  end))

  -- Upgrade the protocol to websocket
  updateDecoder(websocketCodec.decode)
  updateEncoder(websocketCodec.encode)

  split(function ()
    for event in read do
      p(event)
    end
  end, function ()
    for line in getLine do
      write(line)
    end
    write()
  end)

end)()
