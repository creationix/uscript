
local robots = {}
local browsers = {}

local function broadcast(message)
  p("broadcast", message)
  for _, write in pairs(browsers) do
    write(message)
  end
end

local port = require('os').getenv("PORT") or 7000

-- Create TCP server for robots to connect to.
require('coro-net').createServer({
  host = "0.0.0.0",
  port = port + 1
}, function (read, write, socket)
  p(socket:getpeername())
  local data = ""
  local id
  for chunk in read do
    data = data .. chunk
    if #data >= 5 then
      assert(data:byte(1) == 1, "Must be protocol version 1")
      id = string.format("%02x%02x%02x%02x",
        data:byte(2),
        data:byte(3),
        data:byte(4),
        data:byte(5))
      robots[id] = function (code)
        if not code then return write() end
        local len = #code
        write(string.char(
          bit.rshift(len, 8),
          bit.band(len, 0xff)
        ) .. code)
      end
      break
    end
  end
  p("Robot connected: " .. id)
  broadcast({opcode=1,payload=id .. "!"})
  for message in read do
    p("Robot spoke: " .. id)
    broadcast({opcode=1,payload=id .. "@"})
  end
  write()
  robots[id] = nil
  p("Robot disconnected: " .. id)
  broadcast({opcode=1,payload=id .. "#"})
  write()
end)
print("uScript uController endpoint on port 1337")

require('weblit-websocket')
require('weblit-app')

  .bind({
    host = "0.0.0.0",
    port = port
  })

  .use(require('weblit-logger'))
  .use(require('weblit-auto-headers'))
  .use(require('weblit-etag-cache'))

  .use(require('weblit-static')(module.dir .. "/www"))
  .websocket({
    path="/socket",
    protocol="uscript-bridge",
  }, function (req, read, write)
    browsers[req] = write
    print("Browser connected")
    for id in pairs(robots) do
      write({opcode=1,payload=id .. "!"})
    end

    for message in read do
      p(message)
      if message.opcode == 2 then
        local id = string.format("%02x%02x%02x%02x",
          message.payload:byte(1),
          message.payload:byte(2),
          message.payload:byte(3),
          message.payload:byte(4)
        )
        local code = message.payload:sub(5)
        local robot = robots[id]
        if not robot then
          print("No such robot: " .. id)
          return
        end
        p(id, code)
        robots[id](code)
      elseif message.opcode == 1 then
        print("Browser said: " .. message.data)
      end
    end
    print("Browser disconnected")
    browsers[req] = nil
  end)

  .start()
