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
  add: 2, sub: 2, mul: 2, div: 2, mod: 2, neg: 1,
  bnot: 1, bxor: 2, band: 2, bor: 2, lshift: 2, rshift: 2,
  gt: 2, gte: 2, lt: 2, lte: 2, eq: 2, neq: 2,
  srand: 1, rand: 1,
  and: 2, or: 2, xor: 2, not: 1,
  choose: 3,
  isetup: 3,//(pinSDA, pinSCL, speed) -> i2c
  istart: 1,//(i2c) - Send a start sequence.
  istop: 1,//(i2c) - Send a stop sequence.
  iaddr: 3,//(i2c, addr, direction) - Send address and direction.
  iwrite: 2,//(i2c, byte)
  iread: 1,//(i2c) -> byte
  iawrite: 2,//(i2c, buffer)
  iaread: 2,//(i2c, len) -> buffer
};

var symbols = {};

// File loader, eventually will be replaces with git sync system.
function loadFile(path) {
  path = "modules/" + path.replace(/\./g, "/") + ".jack";
  if (!fs.existsSync(path)) { return; }
  return fs.readFileSync(path, "utf8");
}

var loaded = {};
function load(name) {
  // Only load a given scope once.
  if (loaded[name]) { return; }
  loaded[name] = true;
  var parts = name.split(".");
  while (parts.length) {
    var prefix = parts.join(".");
    extract(prefix);
    extract(prefix + ".index");
    parts.pop();
  }
}

var extracted = {};
function extract(scope) {
  // Only extract a given scope once.
  if (extracted[scope]) return;
  extracted[scope] = true;

  // Load the code to process
  var code = loadFile(scope);
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
  }
}


var functions = [];
var nextFn = 0;
function compile(symbol) {
  load(symbol);
  var fn = symbols[symbol];
  if (!fn) return;
  if ("index" in fn) return fn.index;
  console.log("Compiling function " + symbol);

  // First assign the arguments to the first n slots per the calling convention.
  var vars = [];
  var varSlots = {};
  var nargs = fn.args.length;
  fn.args.forEach(function (arg, i) {
    vars[i] = arg;
    varSlots[arg] = i;
  });

  function getSlot(name) {
    var slot = varSlots[name];
    if (slot) { return slot; }
    // TODO: add freelist to reuse empty slots
    slot = vars.length;
    vars[slot] = name;
    varSlots[name] = slot;
    return slot;
  }

  var code = [];
  fn.ast.forEach(onStatement);

  function onStatement(statement) {
    var type = statement[0];
    if (type == "CALL") {
      var name = statement[1];
      var args = statement[2];
      var target = statement[3];
      if (target) {
        target = getSlot(target);
      }
      if (name in builtins) {
        if (args.length !== builtins[name]) {
          throw new Error("Builtin arity mismatch");
        }
        if (target) {
          code.push(["set", target, [name].concat(args.map(walk))]);
        }
        else {
          code.push([name].concat(args.map(walk)));
        }
      }
      else {
        var start = vars.length;
        args.forEach(function (expression, i) {
          expression = walk(expression);
          if (expression[0] == "get") {
            code.push(["copy", i + start, expression[1]]);
          }
          else {
            code.push(["set", i + start, expression]);
          }
        });
        var index = compile(resolveSymbol(fn, name));
        if (target) {
          code.push(["call", index, start, args.length, target]);
        }
        else {
          code.push(["sub", index, start, args.length]);
        }
        vars.length = start;
      }
    }
    else if (type === "ASSIGN") {
      code.push(["set", getSlot(statement[1]), walk(statement[2])]);
    }
    else if (type === "IF") {
      p(statement);
      code.push(["if", walk(statement[1])]);
      statement[2].forEach(onStatement);
      code.push(["end"]);
    }
    else {
      p(statement);
      throw "TODO: handle " + type;
    }
  }

  functions.push(code);
  fn.index = nextFn++;
  fn.code = code;
  p(fn);
  return fn.index;

  function walk(expression) {
    if (!Array.isArray(expression)) { return expression; }
    var type = expression[0].toLowerCase();
    if (type == "ident") {
      var name = expression[1];
      if (name in varSlots) {
        return ["get", varSlots[name]];
      }
      // TODO: add index for values that need data storage
      return symbols[resolveSymbol(fn, name)].val;
    }
    if (type in builtins) {
      if (expression.length !== builtins[type] + 1) {
        throw new Error("Builtin arity mismatch");
      }
      return [type, expression.slice(1).map(walk)];
    }
    else {
      p(expression);
      throw new Error("TODO: handle " + type);
    }
  }
}

// Resolve a symbol to it's full path.
// This will load and parse new files on-demand.
function resolveSymbol(fn, name) {

  // Apply import alias if it matches
  var match = name.match(/^([^.]+)(?:\.(.+))?/);
  var alias = fn.imports[fn.namespace + "." + match[1]];
  if (alias) {
    name = alias + "." + match[2];
  }

  // Look in current all parent scopes for a matching symbol.
  // Stop on first match.
  var parts = fn.namespace.split(".");
  var fullName;
  while (true) {
    var symbol = parts.concat([name]).join(".");
    // Make sure all symbols for this path are loaded already.
    load(symbol);

    if (symbol in symbols) {
      // Check for symbol scope, must be public or matching current scope.
      var sym = symbols[symbol];
      if (sym.pub || sym.scope == fn.scope) {
        return symbol;
      }
    }
    if (!parts.length) {
      throw new Error("Can't find symbol: " + name + " relative to " + fn.namespace);
    }
    parts.pop();
  }
}


console.log("Compiling creationix.test.main");
compile("creationix.test.main");
//compile("creationix.test", "modules/creationix/test.jack");
