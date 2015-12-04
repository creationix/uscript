exports.name = "creationix/weblit-static"
exports.version = "0.3.3-1"
exports.dependencies = {
  "creationix/mime@0.1.2",
  "creationix/hybrid-fs@0.1.1",
}
exports.description = "A weblit middleware for serving static files from disk or bundle."
exports.tags = {"weblit", "middleware", "static"}
exports.license = "MIT"
exports.author = { name = "Tim Caswell" }
exports.homepage = "https://github.com/creationix/weblit/blob/master/libs/weblit-auto-headers.lua"

local getType = require("mime").getType
local jsonStringify = require('json').stringify

local makeChroot = require('hybrid-fs')

return function (rootPath)

  local fs = makeChroot(rootPath)

  return function (req, res, go)
    if req.method ~= "GET" then return go() end
    local path = (req.params and req.params.path) or req.path
    path = path:match("^[^?#]*")
    if path:byte(1) == 47 then
      path = path:sub(2)
    end
    local stat = fs.stat(path)
    if not stat then return go() end

    local function renderFile()
      local body = assert(fs.readFile(path))
      res.code = 200
      res.headers["Content-Type"] = getType(path)
      res.body = body
      return
    end

    local function renderDirectory()
      if req.path:byte(-1) ~= 47 then
        res.code = 301
        res.headers.Location = req.path .. '/'
        return
      end
      local files = {}
      for entry in fs.scandir(path) do
        if entry.name == "index.html" and entry.type == "file" then
          path = (#path > 0 and path .. "/" or "") .. "index.html"
          return renderFile()
        end
        files[#files + 1] = entry
        entry.url = "http://" .. req.headers.host .. req.path .. entry.name
      end
      local body = jsonStringify(files) .. "\n"
      res.code = 200
      res.headers["Content-Type"] = "application/json"
      res.body = body
      return
    end

    if stat.type == "directory" then
      return renderDirectory()
    elseif stat.type == "file" then
      if req.path:byte(-1) == 47 then
        res.code = 301
        res.headers.Location = req.path:match("^(.*[^/])/+$")
        return
      end
      return renderFile()
    end
  end
end
