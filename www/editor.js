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

  var xhr = new XMLHttpRequest();
  xhr.open("GET", "dancebot.uscript", true);
  xhr.overrideMimeType("text/plain");
  xhr.addEventListener("load", function () {
    editor.setValue(xhr.responseText);
    reindent();
  });
  xhr.send();

  function reindent() {
    for (var i = 0, l = editor.lineCount(); i < l; i++) {
      editor.indentLine(i);
    }
  }

  var robotsDiv = document.querySelector(".robots");
  var robots = {};
  function addRobot(id) {
    var img = document.createElement("img");
    var hash = md5(id.toString(15));
    img.setAttribute('src',
      'data:image/png;base64,' + new Identicon(hash, 420));
    robotsDiv.appendChild(img);
    robots[id] = img;
    img.onclick = function () {
      socket.send(compile(id, editor.getValue()));
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
  socket.binaryType = 'arraybuffer';
  socket.onmessage = function(evt) {
    if (typeof evt.data === 'string') throw evt.data;
    var message = new Uint8Array(evt.data);
    var id = (message[0] << 24)
           | (message[1] << 16)
           | (message[2] << 8)
           | (message[3]);
    var command = message[4];
    if (command === 0) {
      console.log("Remove robot", id);
      removeRobot(id);
    }
    else if (command === 1) {
      console.log("Add robot", id);
      addRobot(id);
    }
    else if (command === 2) {
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

  function compile(id, text) {
    console.log(text);
    return new Uint8Array([
      id >> 24,
      (id >> 16) & 0xff,
      (id >> 8) & 0xff,
      id & 0xff,
      1,2,3,4]);
  }
};
