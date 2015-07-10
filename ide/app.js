var docs = {
  def: ["Define", "Define a program to be run later"],
  while: ["While", "While condition is not zero, repeat loop body"],
  pm: ["Pin Mode", "Set the GPIO mode to 0-Input or 1-Output"],
  dw: ["Digital Write", "Set an output pin to 0-Off or 1-On"],
  dr: ["Digital Read", "Read the current value of a pin as 0-Off, 1-On"],
  ar: ["Analog Read", "Read the current value of a pin as analog."],
  not: ["Not", "Invert value.  0 -> 1 and non-zero to zero"],
  delay: ["Delay", "Delay for a number of milliseconds"],
  add: ["+", "Add two values", 1],
  sub: ["-", "Subtract two values", 1],
  mul: ["×", "Multiply two values", 1],
  div: ["÷", "Divide two values", 1],
  lt: ["<", "1 if left is less than right, otherwise 0", 1],
  lte: ["≤", "1 if left is less or equal to right, otherwise 0", 1],
  gt: [">", "1 if left is greater than right, otherwise 0", 1],
  gte: ["≥ ", "1 if left is greater than or equal to right, otherwise 0", 1],
  set: ["◀", "Store a value in a memory slot", 1],
  get: ["", "Look up value in slot"],
  mod: ["mod", "Get the modulus (remainder)", 1],
  abs: ["|", "Find the absolute value of a value", 2],
}

// DEF a DO 3 PM 4 1 PM a0 0 WHILE 1 DO 2 DW 4 NOT DR 4 DELAY AR a0
var def = ["def", 0,
  ["do", 3,
    ["pm", 4, 1],
    ["pm", 17, 0],
    ["while", 1,
      ["do", 2,
        ["dw", 4,
          ["not",
            ["dr", 4]
          ]
        ],
        ["delay",
          ["ar", 17]
        ]
      ]
    ]
  ]
];

// DEF 17 DO 7
//   SET 2 SUB 512 ABS SUB GET 11 512
//   SET 23 DIV MUL SUB 512 ABS SUB MOD GET 7 1024 512 GET 2 512
//   IF LT GET 7 512 DO 3
//     SET 17 GET 2
//     SET 6 GET 23
//     SET 1 0
//   ELIF LT GET 7 1024 DO 3
//     SET 17 GET 23
//     SET 6 GET 2
//     SET 1 0
//   ELIF LT GET 7 1536 DO 3
//     SET 17 0
//     SET 6 GET 2
//     SET 1 GET 23
//   ELIF LT GET 7 2048 DO 3
//     SET 17 0
//     SET 6 GET 23
//     SET 1 GET 2
//   ELIF LT GET 7 2560 DO 3
//     SET 17 GET 23
//     SET 6 0
//     SET 1 GET 2
//   ELSE DO 3
//     SET 17 GET 2
//     SET 6 0
//     SET 1 GET 23
//   SET 12 DIV SUB GET 11 GET 2 2
//   SET 17 ADD GET 17 GET 12
//   SET 6 ADD GET 6 GET 12
//   SET 1 ADD GET 1 GET 12

var def2 = ["def", 17, ["do", 7,
  ["set", 2, ["sub", 512, ["abs", ["sub", ["get", 11], 512]]]],
  ["set", 23, ["div", ["mul", ["sub", 512, ["abs", ["sub", ["mod", ["get", 7], 1024], 512]]], ["get", 2], ],512]],
  ["if", ["lt", ["get", 7], 512], ["do", 3,
    ["set", 17, ["get", 2]],
    ["set", 6, ["get", 23]],
    ["set", 1, 0],
  ]],
  ["elif", ["lt", ["get", 7], 1024], ["do", 3,
    ["set", 17, ["get", 23]],
    ["set", 6, ["get", 2]],
    ["set", 1, 0],
  ]],
  ["elif", ["lt", ["get", 7], 1536], ["do", 3,
    ["set", 17, 0],
    ["set", 6, ["get", 2]],
    ["set", 1, ["get", 23]],
  ]],
  ["elif", ["lt", ["get", 7], 2048], ["do", 3,
    ["set", 17, 0],
    ["set", 6, ["get", 23]],
    ["set", 1, ["get", 2]],
  ]],
  ["elif", ["lt", ["get", 7], 2560], ["do", 3,
    ["set", 17, ["get", 23]],
    ["set", 6, 0],
    ["set", 1, ["get", 2]],
  ]],
  ["else", ["do", 3,
    ["set", 17, ["get", 2]],
    ["set", 6, 0],
    ["set", 1, ["get", 23]],
  ]],
  ["set", 12, ["div", ["sub", ["get", 11], ["get", 2]], 2]],
  ["set", 17, ["add", ["get", 17], ["get", 12]]],
  ["set", 6, ["add", ["get", 6], ["get", 12]]],
  ["set", 1, ["add", ["get", 1], ["get", 12]]]
]];
function render(ast, depth) {
  if (typeof ast === "number") {
    var node = document.createElement("div");
    node.setAttribute("class", "node number d" + depth);
    node.textContent = ast;
    return node;
  }
  var node, i;
  var mode
  if (ast[0] === "do") {
    node = document.createDocumentFragment();
    i = 2;
  }
  else {
    node = document.createElement("div");
    var type = ast[0];
    node.setAttribute("class", "node " + type + " d" + ((depth + 1)%16));
    var doc = docs[type];
    mode = doc && doc[2];
    var name = doc ? doc[0] : type;
    var label = document.createTextNode(name);
    if (doc && doc[1]) {
      node.setAttribute("title", doc[1]);
    }
    node.appendChild(label);
    i = 1;
    if (type === "get" || type === "set" || type == "def") {
      i = 2;
      var child = document.createElement("div");
      child.setAttribute("class", "node variable d" + ((depth + 2)%16));
      child.textContent = String.fromCharCode(65 + ast[1]);
      node.appendChild(child);
    }
    depth = (depth + 2) % 16;
  }
  for (var l = ast.length; i < l; i++) {
    node.appendChild(render(ast[i], depth));
  }
  if (mode === 1) {
    node.insertBefore(node.childNodes[1], node.childNodes[0]);
  }
  if (mode === 2) {
    node.appendChild(node.childNodes[0].cloneNode());
  }
  return node;
}

window.onload = function () {
  document.body.appendChild(render(def2, Math.floor(Math.random() * 16)));
};
