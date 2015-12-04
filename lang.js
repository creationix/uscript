
// Print enum list for C code.
//console.log("enum opcode {\n  " + Object.keys(BUILTINS).join(", ") + "\n};");

var code = "ADD 1 2 END";


var keyword = new RegExp("^(" + Object.keys(BUILTINS).join("|") + ")");
console.log(code);
console.log(code.match(keyword));
