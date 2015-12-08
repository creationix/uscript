(function () {
  var opcodes = [ "do", "end",  "return", "dump",  "mode",  "read",  "write",
  "aread", "pwrite",  "ibegin",  "ifrom",  "istart",  "istop",  "iwrite",
  "iavailable", "iread",  "tone",  "delay",  "call",  "gosub",  "goto",  "gget",
  "gset", "get",  "set",  "incr", "decr",  "incrmod", "decrmod",  "forever",
  "while", "wait",  "if",  "elseif",  "else",  "add", "sub", "mul", "div",
  "mod", "neg", "band", "bor", "bxor", "bnot", "lshift", "rshift", "and", "or",
  "xor", "not", "choose", "gt", "gte", "lt", "lte", "eq", "neq", "srand",
  "rand",  "restart", "chipid", "flashchipid", "cyclecount", "getfree" ];

  var map = {};
  opcodes.forEach(function (name, i) {
    map[name] = i + 128;
  });

  var lexers = [
    "whitespace", /^[ \r\n\t]+/,
    "comment", /^[-][-].*/,
    "var", /^var\b/,
    "set", /^set\b/,
    "get", /^(get|incr|decr|incrmod|decrmod)\b/,
    "const", /^const\b/,
    "jump", /^(call|gosub|goto)\b/,
    "opcode", new RegExp("^(" + opcodes.sort().join("|") + ")\\b"),
    "number", /^[-]?(?:0b[01]+|0o[07]+|0x[0-9a-f]+|[1-9][0-9]*|0)\b/i,
    "label", /^[a-z_$]\w*:/i,
    "ident", /^[a-z_$]\w*/i,
    "error", /^./,
  ];



  function compile(text) {

    var bytes = [];
    var variables = {};
    var consts = {};
    var labels = {};
    var nextIndex = 0;
    var labelQueue = [];


    function nextToken(expectedType) {
      for (var i = 0, l = lexers.length; i < l; i += 2) {
        var lexer = lexers[i + 1];
        var match = text.match(lexer);
        if (!match) continue;
        var type = lexers[i];
        var token = match[0];
        text = text.substring(token.length);
        while (type === "ident" && (token in consts)) {
          type = consts[token].type;
          token = consts[token].token;
        }

        if (type === "whitespace" || type === "comment") {
          return nextToken(expectedType);
        }
        if (expectedType) {
          if (type !== expectedType) {
            throw new Error("Expected " + expectedType + " but got " + type + ": " + token);
          }
          return token;
        }
        return {type:type, token:token};
      }
    }

    var next;
    while ((next = nextToken())) {

      var type = next.type,
          token = next.token;

      switch (type) {
        case "whitespace": case "comment": continue;
        case "error": throw new Error("Umexpected input: " + token);
        case "var":
          var name = nextToken("ident");
          if (name in consts || name in variables) throw new Error("Name conflict: " + name);
          bytes.push(map.set, variables[name] = nextIndex++);
          continue;
        case "set":
          var name = nextToken("ident");
          if (!(name in variables)) throw new Error("undefined variable: " + name);
          bytes.push(map.set, variables[name]);
          continue;
        case "get":
          var name = nextToken("ident");
          if (!(name in variables)) throw new Error("undefined variable: " + name);
          bytes.push(map[token], variables[name]);
          continue;
        case "const":
          var name = nextToken("ident");
          if (name in consts || name in variables) throw new Error("Name conflict: " + name);
          consts[name] = nextToken();
          continue;
        case "number":
          token = Number(token);
          var list = [];
          var neg = false;
          var more = 0;
          if (token < 0) {
            token = -token;
            neg = true;
          }
          while (token >= 0x40) {
            list.unshift((token & 0x7f) | more);
            token >>= 7;
            more = 0x80;
          }
          list.unshift(token | (more >> 1));
          if (neg) list.unshift(map.neg);
          bytes.push.apply(bytes, list);
          continue;
        case "jump":
          bytes.push(map[token]);
          var name = nextToken("ident");
          if (name in labels) {
            var offset = labels[name] - (bytes.length + 2);
            bytes.push((offset >>> 8) & 0xff, offset & 0xff);
          }
          else {
            bytes.push(0,0);
            labelQueue.push({name: name,offset: bytes.length});
          }
          continue;
        case "label":
          token = token.substring(0, token.length - 1);
          labels[token] = bytes.length;
          for (var i = labelQueue.length - 1; i >= 0; i--) {
            var item = labelQueue[i];
            if (item.name !== token) continue;
            labelQueue.splice(i, 1);
            var jmp = bytes.length - item.offset;
            bytes[item.offset - 2] = (jmp >> 8) & 0xff;
            bytes[item.offset - 1] = jmp & 0xff;
          }
          continue;
        case "ident":
          if (token in labels) {
            throw "CALC";
          }
          else if (token in variables) {
            bytes.push(map.get, variables[token]);
          }
          continue;
        case "opcode":
          bytes.push(map[token]);
          continue;
        default: {
          console.log(next);
          throw "TODO:"
        }
      }

    }
    return bytes;
  }
  window.compile = compile;
})();
