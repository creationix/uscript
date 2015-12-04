var inspect = require('util').inspect;
function p(val) {
  console.log(inspect(val, {colors:true,depth:null}));
}
global.p = p;
var parser = require('./www/parser');
var opcodes = require('./www/opcodes');
var patterns = require('./www/patterns');

p(parser);
p(opcodes);
p(patterns);
