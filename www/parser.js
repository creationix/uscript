(function (def) {
  try { module.exports = def(require('./opcodes')); }
  catch (err) { window.parser = def(window.opcodes); }
})(function (opcodes) {
  return function parser(state, type, token) {
    "use strict";
    var stack = state.stack;
    if (!stack.length) return "error";
    var top = stack[stack.length - 1];
    // console.log(type, token, stack.length, top);

    function cloneScope() {
      var scope = {};
      var keys = Object.keys(top.scope);
      for (var i = 0, l = keys.length; i < l; i++) {
        var key = keys[i];
        scope[key] = top.scope[key];
      }
      return scope;
    }

    // Handle the function name token
    if (top.mode === "function") {
      // The token after FUN must be a new, unused variable
      if (type !== "ident" || top.scope[token] !== undefined) return "error";
      // Prepare to consume args list
      top.mode = "args";
      top.name = token;
      top.args = [];
      return "function";
    }

    // Handle the function args list and trailing DO
    if (top.mode === "args") {
      if (type === "opcode" && token === "do") {
        top.scope[top.name] = top.args.length;
        for (var i = 0, l = top.args.length; i < l; i++) {
          top.scope[top.args[i]] = "var";
        }
        delete top.args;
        top.mode = "block";
        top.expectToken = "end";
        top.expectType = "opcode";
        return type;
      }
      if (type !== "ident" || top.scope[token] !== undefined) return "error";
      top.args.push(token);
      return "variable";
    }

    function pop() {
      // Check for export values
      var name, value;
      if (top.name) {
        name = top.name;
        value = top.scope[name];
      }
      // Clean up the old stack frame
      stack.pop();
      top = stack[stack.length - 1];

      // Export the value if there was one
      if (name) {
        top.scope[name] = value;
      }
    }

    // Ensure the first token after DEF and LET are unused variables.
    if (top.type === "const" || top.type === "var" || top.type === "set" ||
        top.type === "call" || top.type === "gosub" || top.type === "goto") {
      if (type !== "ident") return "error";
    }

    // Handle a block closer
    if (typeof top.left === "number" && token !== top.expectToken) {
      top.left--;
      if (top.left < 0) return "error";
    }

    if (type === "ident") {
      var kind;
      if (top.type === "call" || top.type === "gosub" || top.type === "goto") {
        type = "label";
      }
      else {
        if (top.type === "const" || top.type === "var") {
          top.scope[token] = top.scope[token] === undefined ? (
            top.type === "const" ? "const" : "var") : "error";
          if (top.type === "const" || top.type === "var") {
            top.name = token;
          }
        }
        kind = top.scope[token];

        // Figure out which kind of variable this is.
        if (kind === "var") type = "variable";
        else if (kind === "const" && top.type !== "set") type = "definition";
        else if (typeof kind === "number" && top.type !== "set") {
          type = "function";
        }
        else type = "error";
      }

      if (top.type) delete top.type;
      if (type === "function" && kind > 0) {
        stack.push({
          mode: "fixed",
          scope: cloneScope(),
          left: kind,
        });
        return type;
      }
    }
    else if (type === "bracket") {
      if (token === "(" || token === "[" || token === "{" || token === "<") {
        stack.push(top = {
          mode: "block",
          scope: cloneScope(),
          expectType: "bracket",
          expectToken: {
            "(":")",
            "[":"]",
            "{":"}",
            "<":">",
          }[token],
        });
        if (token === "(") {
          top.left = 2;
        }
        return type;
      }
      if (top.mode !== "block" || top.expectType !== "bracket") {
        return "error";
      }
    }

    else if (type === "opcode") {
      if (token === "def") {
        stack.push({
          mode: "function",
          scope: cloneScope(),
        });
        return type;
      }
      else if (token === "do") {
        stack.push({
          mode: "block",
          scope: cloneScope(),
          expectToken: "end",
          expectType: "opcode",
        });
        return type;
      }
      else if (opcodes[token]) {
        stack.push({
          mode: "fixed",
          scope: cloneScope(),
          left: opcodes[token],
          type: token
        });
        return type;
      }
    }

    if (top.mode === "block" && type === top.expectType && token === top.expectToken) {
      if (typeof top.left === "number" && top.left !== 0) {
        type = "error";
      }
      pop();
    }
    // TODO: somehow show elseif and else as same level as associated if
    // Clean out any filled fixed items.
    while (top.mode === "fixed" && !top.left) {
      pop();
    }
    return type;
  };
});
