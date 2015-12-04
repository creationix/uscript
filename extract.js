/*jshint esnext:true */
// `symbols` is a simple map from name to value for all symbols.
//   The name is in the form "foo.bar.baz".  The value is an object containing:
//    - scope - the file that defined this symbol
//    - pub - a boolean flagging if this symbol can be used outside it's scope
//    - type (constant, function, buffer, structure)
//    - value (for constants, buffers, and structures)
//    - ast (for functions)
//    - args (for functions)
//    - index (for compiled functions and used buffers or structures)
// `loader` is a simple generator function that takes a path and returns a
// string if the file exists and nothing if not.
// `parsers` is a map of file extensions to jison parser instances.
module.exports = function extract(symbols, loader, parsers) {
  "use strict";

  var extracted = {};

  // Given a file path in a.b.c.d form, load an parse the file and dump The
  // symbols in the global symbol table.
  return function* (path) {
    // Ensure the same path is only extracted once.
    if (extracted[path]) return;
    extracted[path] = true;

    var code = yield* loader(path);
    // If the file doesn't exist, there is nothing to do.  We're done here.
    if (!code) return;

    // Look up the parser and namespace for this source file.
    var match = path.match(/^(.+)\.([^.]+)$/);
    var parse = parsers[match[2]];
    var namespace = match[1].replace(/\//g, ".");
    match = namespace.match(/^(.*)\.index$/);
    if (match) namespace = match[1];

    var ast = parse(code);

    p(namespace);
    p(ast);
  };
};
