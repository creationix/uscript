/* jshint node:true*/
"use strict";
var fs = require('fs');
var input = "./grammar.jison";
var output = "./parser.js";
var parser;
if (!fs.existsSync(output) ||
    fs.statSync(input).mtime > fs.statSync(output).mtime) {
  console.log("Generating new parser.js");
  var Parser = require("jison").Parser;
  parser = new Parser(fs.readFileSync(input, "utf8"));
  fs.writeFileSync(output, parser.generate());
}
else {
  parser = require(output).parser;
}
var inspect = require('util').inspect;
function p(val) {
  console.log(inspect(val, {colors:true,depth:null}));
}

var builtins = {
  isetup: 3
};
var symbols = {};

// File loader, eventually will be replaces with git sync system.
function load(path) {
  path = "modules/" + path.replace(/\./g, "/") + ".jack";
  if (!fs.existsSync(path)) { return; }
  return fs.readFileSync(path, "utf8");
}

var extracted = {};
function extract(scope) {
  // Only extract a given scope once.
  if (extracted[scope]) return;
  extracted[scope] = true;

  // Load the code to process
  var code = load(scope);
  // If it doesn't exist, we're done.
  if (!code) { return; }

  // Make sure to strip trailing .index parts for namespace purposes
  var match = scope.match(/^(.*)\.index$/);
  var namespace = match ? match[1] : scope;

  console.log("Extracting symbols from " + scope);

  // Remember import aliases for this file
  var imports = {};

  // Parse the text and process the top-level statements.
  parser.parse(code).forEach(function (line) {
    process(namespace, line);
  });

  function process(namespace, statement) {
    function getName(name) {
      name = namespace + "." + name;
      if (name in symbols || name in builtins || name in imports) {
        throw new Error("Duplicate symbol: " + name);
      }
      return name;
    }
    var type = statement[0];

    // Import statements simply record the alias.
    if (type === "IMPORT") {
      var last = statement[1].match(/[^.]+$/)[0];
      imports[getName(last)] = statement[1];
    }

    // Constants store their value in the table.
    else if (type == "CONSTANT") {
      var sym = symbols[getName(statement[2])] = { val: statement[3] };
      if (statement[1]) { sym.pub = true; }
      else { sym.scope = scope; }
    }

    // Functions store enough context to be able to compile later.
    else if (type == "FUNCTION") {
      var fn = symbols[getName(statement[2])] = {
        imports: imports,
        namespace: namespace,
        scope: scope,
        args: statement[3],
        ast: statement[4],
      };
      if (statement[1]) {
        fn.pub = true;
      }
    }
    else if (type == "NAMESPACE") {
      statement[2].forEach(function (line) {
        process(namespace + "." + statement[1], line);
      });
    }
    else {
      p(statement);
      throw "TODO: handle " + type;
    }
  }
}

function find(name) {
  if (name in symbols) { return; }
  var parts = name.split(".");
  while (parts.length) {
    var prefix = parts.join(".");
    extract(prefix);
    extract(prefix + ".index");
    parts.pop();
  }
}

var functions = [];
var nextFn = 0;
function apply(fn) {
  if (fn.code) { return fn; }
  var vars = {};
  var nvars = fn.args.length;
  fn.args.forEach(function (arg, i) {
    vars[arg] = nvars - i;
  });
  var code = [];
  fn.ast.forEach(function (statement) {
    var type = statement[0];
    if (type == "CALL") {
      var name = statement[1];
      var args = statement[2];
      var target = statement[3];
      if (name in builtins) {
        if (args.length !== builtins[name]) {
          throw new Error("Builtin arity mismatch");
        }
        code.push([name].concat(args.map(walk)));
      }
      else {
        code.push(["call", resolveFn(name)].concat(args.map(walk)));
      }
      if (target) {
        if (!(target in vars)) {
          vars[target] = ++nvars;
        }
        code.push(["set", vars[target], 0]);
      }
    }
    else {
      p(statement);
      throw "TODO: handle " + type;
    }
  });

  var diff = nvars - fn.args.length;
  if (diff) {
    code.unshift(["alloc", diff]);
    code.push(["free", diff]);
  }
  functions.push(code);
  return { index: nextFn++ };

  function walk(expression) {
    if (!Array.isArray(expression)) { return expression; }
    var type = expression[0];
    if (type == "IDENT") {
      return resolve(expression[1]);
    }
  }

  function searchNamespace(namespace, name) {
    var parts = namespace.split(".");
    while (parts.length) {
      var symbol = parts.join(".") + "." + name;
      if (symbol in symbols) {
        var sym = symbols[symbol];
        if (sym.pub || sym.scope == fn.scope) {
          return sym;
        }
      }
      parts.pop();
    }
  }

  function search(name) {
    var sym = searchNamespace(fn.namespace, name);
    if (sym) return sym;
    var namespaces = [name + " in " + fn.namespace];
    var match = name.match(/^([^.]+)(?:\.(.+))?/);
    var alias = fn.imports[fn.namespace + "." + match[1]];
    if (alias) {
      namespaces.push(match[2] + " in " + alias);
      find(alias + "." + match[2]);
      sym = searchNamespace(alias, match[2]);
      if (sym) return sym;
    }
    throw new Error("Can't find symbol: " + namespaces.join(" or "));
  }

  function resolve(name) {
    if (name in vars) {
      return ["get", vars[name]];
    }
    // TODO: add index for values that need data storage
    return search(name).val;
  }

  function resolveFn(name) {
    var sym = search(name);
    p(sym);
  }

}


find("creationix.test");
console.log("Compiling creationix.test.main");
symbols["creationix.test.main"] = apply(symbols["creationix.test.main"]);
p(functions);
//compile("creationix.test", "modules/creationix/test.jack");
