exports.name = "creationix/hybrid-fs"
exports.version = "0.1.1"
exports.dependencies = {
  "creationix/coro-fs",
}
exports.license = "MIT"
exports.description = "A common filesystem subset that works inside or outside luvi bundles"
exports.tags = {"fs", "bundle"}
exports.author = { name = "Tim Caswell" }
exports.homepage = "https://github.com/creationix/hybrid-fs/"

local makeChroot = require('coro-fs').chroot
local pathJoin = require('luvi').path.join
local bundle = require('luvi').bundle

return function (root)
  local match = root:match("^bundle:/?(.*)$")
  if not match then return makeChroot(root) end
  root = match

  local fs = {}
  function fs.scandir(path)
    path = pathJoin(root, './' .. path)
    local names = bundle.readdir(path)
    local i = 0
    return function ()
      i = i + 1
      local name = names[i]
      if not name then return end
      local stat = bundle.stat(pathJoin(path, name))
      stat.name = name
      return stat
    end
  end

  function fs.readFile(path)
    path = pathJoin(root, './' .. path)
    return bundle.readfile(path)
  end

  function fs.stat(path)
    path = pathJoin(root, './' .. path)
    return bundle.stat(path)
  end

  return fs
end
