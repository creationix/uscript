window.onload = function () {
  "use strict";


  var editor = CodeMirror.fromTextArea(document.querySelector("textarea"), {
    mode: "uscript-asm",
    keyMap: "sublime",
    matchBrackets: true,
    showCursorWhenSelecting: true,
    tabSize: 2,
    extraKeys: {
      "Ctrl-Q": function(cm) {
        cm.foldCode(cm.getCursor());
      },
      "Ctrl-I": reindent,
    },
    foldGutter: true,
    lineNumbers: true,
    gutters: ["CodeMirror-linenumbers", "CodeMirror-foldgutter"],
    theme: "ambiance",
    autoCloseBrackets: {
      pairs: "()[]{}<>",
      explode: "()[]{}<>",
    },
  });

  var code = localStorage.getItem("code");
  if (code) {
    editor.setValue(code);
    reindent();
  }
  else {
    loadFile("dancebot.uscript");
  }
  editor.on("change", function () {
    localStorage.setItem("code", editor.getValue());
  });

  function loadFile(name) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", name, true);
    xhr.overrideMimeType("text/plain");
    xhr.addEventListener("load", function () {
      editor.setValue(xhr.responseText);
      reindent();
    });
    xhr.send();
  }



  function reindent() {
    for (var i = 0, l = editor.lineCount(); i < l; i++) {
      editor.indentLine(i);
    }
  }

  var robotsDiv = document.querySelector(".robots");
  var robots = {};

  [
    {file:"dancebot.uscript",title:"Dancebot"},
    {file:"tri-switch.uscript",title:"TriSwitch"},
    {file:"sparkle.uscript",title:"Sparkle"},
  ].forEach(function (data) {
    var button = document.createElement("button");
    button.textContent = data.title;
    button.onclick = function () {
      loadFile(data.file);
    };
    robotsDiv.appendChild(button);
  });


  function addRobot(id) {
    var img = document.createElement("img");
    var hash = md5(id.toString(15));
    img.setAttribute('src',
      'data:image/png;base64,' + new Identicon(hash, 420));
    robotsDiv.appendChild(img);
    robots[id] = img;
    img.onclick = function () {
      socket.send(compileForBot(id, editor.getValue()));
    };
  }


  function removeRobot(id) {
    var img = robots[id];
    if (!img) { return; }
    delete robots[id];
    robotsDiv.removeChild(img);
  }

  var url = ("" + window.location.origin).replace(/^http/, "ws") + "/socket";
  var socket = new WebSocket(url, "uscript-bridge");
  socket.onmessage = function(evt) {
    var id = evt.data.substring(0, 8);
    var command = evt.data[8];
    var message = evt.data.substring(9);
    if (command === "#") {
      console.log("Remove robot", id);
      removeRobot(id);
    }
    else if (command === '!') {
      console.log("Add robot", id);
      addRobot(id);
    }
    else if (command === "#") {
      console.log("Robot said", message);
    }
  };


  document.querySelector(".fullscreen").onclick = function () {
    var elem = document.body;
    if (elem.requestFullscreen) {
      elem.requestFullscreen();
    } else if (elem.msRequestFullscreen) {
      elem.msRequestFullscreen();
    } else if (elem.mozRequestFullScreen) {
      elem.mozRequestFullScreen();
    } else if (elem.webkitRequestFullscreen) {
      elem.webkitRequestFullscreen();
    }
  };

  // var samples = {
  //   '00f1cf9d': ([
  //     0x80,0x83,0x05,0x01,0x83,0x06,0x01,0x83,0x07,0x01,0x83,0x08,0x01,0x85,0x05,0x00,0x85,0x06,0x00,0x85,0x07,0x00,0x85,0x08,0x00,0x88,0x02,0x01,0x8a,0x40,0x70,0x8c,0x21,0x8b,0x00,0x8a,0x40,0x70,0x8c,0x41,0x01,0x8b,0x00,0x8a,0x40,0x70,0x8c,0x41,0x6f,0x8b,0x00,0x97,0x00,0x00,0x9c,0x80,0x85,0x08,0x00,0x85,0x05,0x01,0x92,0x1a,0x85,0x05,0x00,0x85,0x06,0x01,0x92,0x12,0x85,0x06,0x00,0x85,0x07,0x01,0x92,0x0a,0x85,0x07,0x00,0x85,0x08,0x01,0x92,0x02,0x81,0x81,0x80,0x97,0x03,0x41,0x00,0x9d,0x99,0x03,0x80,0x8a,0x40,0x70,0x8c,0x9a,0x00,0x10,0x8c,0xba,0x42,0x00,0x8b,0x00,0x90,0x03,0x81,0x81
  //   ]),
  //   '00a63f3d': ([
  //     0x80,0x83,0x07,0x01,0x88,0x02,0x01,0x8a,0x40,0x70,0x8c,0x21,0x8b,0x00,0x8a,0x40,0x70,0x8c,0x41,0x01,0x8b,0x00,0x8a,0x40,0x70,0x8c,0x41,0x6f,0x8b,0x00,0x97,0x00,0x00,0x9c,0x80,0x97,0x01,0x08,0x9d,0x96,0x01,0x80,0x99,0x01,0x8a,0x40,0x70,0x8c,0xa4,0x96,0x01,0x02,0x8c,0xac,0x01,0x96,0x00,0x8b,0x00,0x97,0x00,0xa6,0xa2,0x96,0x00,0xba,0x02,0x08,0x81,0x97,0x00,0xa6,0xa2,0x96,0x00,0x04,0x08,0x8f,0x07,0xa2,0x43,0x68,0xba,0x4f,0x68,0x1e,0x81,0x81
  //   ]),
  //   '00a640a9': ([
  //     0x80,0x83,0x05,0x01,0x83,0x06,0x01,0x83,0x07,0x01,0x83,0x08,0x01,0x85,0x08,0x00,0x83,0x01,0x02,0x83,0x02,0x02,0x97,0x02,0x02,0x97,0x03,0x01,0x97,0x00,0x00,0x9c,0x80,0x9f,0xb1,0x84,0x01,0x80,0x9a,0x00,0x06,0x92,0x3d,0x90,0x41,0x00,0x9e,0x84,0x01,0x90,0x41,0x00,0x81,0xa0,0xb1,0x84,0x02,0x80,0x9b,0x00,0x06,0x92,0x29,0x90,0x41,0x00,0x9e,0x84,0x02,0x90,0x41,0x00,0x81,0x97,0x02,0xa2,0x96,0x02,0x96,0x03,0x9f,0xb4,0x96,0x02,0x41,0x00,0x97,0x03,0xa7,0x01,0xa0,0xb6,0x96,0x02,0x01,0x97,0x03,0x01,0x92,0x04,0x90,0x0a,0x81,0x81,0x80,0x87,0x07,0xa4,0x96,0x02,0xaf,0xb6,0x96,0x00,0x01,0xb4,0x96,0x00,0x05,0x87,0x06,0xa4,0x96,0x02,0xae,0xb4,0x96,0x00,0x01,0xb6,0x96,0x00,0x03,0x87,0x05,0xa4,0x96,0x02,0xb4,0x96,0x00,0x03,0x81
  //   ]),
  // };
  function compileForBot(id, text) {
    var bytecode = compile(text);
    console.log(bytecode);
    return new Uint8Array([
      parseInt(id.substring(0,2), 16),
      parseInt(id.substring(2,4), 16),
      parseInt(id.substring(4,6), 16),
      parseInt(id.substring(6,8), 16),
    ].concat(bytecode));
  }
};
