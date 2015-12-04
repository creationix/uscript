var fs = require('fs');

module.exports = function* loader(path) {
  path = "modules/" + path;
  return yield function (cb) {
    fs.readFile(path, "utf8", function (err, data) {
      if (err) {
        if (err.code === "ENOENT") {
          return cb();
        }
        return cb(err);
      }
      return cb(null, data);
    });
  };
}
