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
  parser = require(output);
}

console.log(parser.parse("export num = 1 - 2"));
