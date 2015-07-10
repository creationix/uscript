var docs = {
  def: ["Define", "Define a program to be run later"],
  while: ["While", "While condition is not zero, repeat loop body"],
  pm: ["Pin Mode", "Set the GPIO mode to 0-Input or 1-Output"],
  dw: ["Digital Write", "Set an output pin to 0-Off or 1-On"],
  dr: ["Digital Read", "Read the current value of a pin as 0-Off, 1-On"],
  ar: ["Analog Read", "Read the current value of a pin as analog."],
  aw: ["Analog Write", "Analog write to a pin.  Really it's PWM write."],
  run: ["Run", "Run a subroutine and return back to here"],
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
  "do": [""],
}

function render(ast, depth) {
  if (typeof ast === "number") {
    var node = document.createElement("div");
    node.setAttribute("class", "node number d" + depth);
    node.textContent = ast;
    return node;
  }
  var node, i;
  var mode
  if (ast[0] === false) {
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
    if (type === "get" || type === "set" || type == "def" || type == "run") {
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

var color = Math.floor(Math.random() * 16);
function add(def) {
  color = (color + 3) % 16;
  document.body.appendChild(render(def, color));
}

window.onload = function () {

  // DEF 0 DO 8
  //   PM 12 1
  //   PM 13 1
  //   PM 14 1
  //   PM 5 0
  //   PM 4 0
  //   SET 11 512
  //   RUN 15
  //   WHILE 1 RUN 11
  add(["def", 0, ["do", 8,
    ["pm", 12, 1],
    ["pm", 13, 1],
    ["pm", 14, 1],
    ["pm", 5, 0],
    ["pm", 4, 0],
    ["set", 11, 512],
    ["run", 15],
    ["while", 1, ["run", 11]]
  ]]);

  // DEF 11 DO 4
  //   IF DR 5
  //     SET 7 MOD ADD GET 7 1 3072
  //   IF DR 4
  //     SET 11 MOD ADD GET 11 1 1024
  //   RUN 15
  //   DELAY 3
  add(["def", 11, ["do", 4,
    ["if", ["dr", 5],
      ["set", 7, ["mod", ["add", ["get", 7], 1]], 3072]],
    ["if", ["dr", 4],
      ["set", 11, ["mod", ["add", ["get", 11], 1]], 1024]],
    ["run", 15],
    ["delay", 3]
  ]]);

  // DEF 15 DO 4
  //   RUN 17
  //   AW 12 SUB 1023 GET 17
  //   AW 13 SUB 1023 GET 6
  //   AW 14 SUB 1023 GET 1

  add(["def", 15, ["do", 4,
    ["run", 17],
    ["aw", 12, ["sub", 1023, ["get", 17]]],
    ["aw", 13, ["sub", 1023, ["get", 6]]],
    ["aw", 14, ["sub", 1023, ["get", 1]]],
  ]]);

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
  add(["def", 17, ["do", 7,
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
  ]]);

  // DEF a DO 3 PM 4 1 PM a0 0 WHILE 1 DO 2 DW 4 NOT DR 4 DELAY AR a0
  add(["def", 25,
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
  ]);

};
