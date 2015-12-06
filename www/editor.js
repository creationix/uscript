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
  xhr.open("GET", "tri-switch.uscript", true);
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


  var elem = document.querySelector(".CodeMirror");
  elem.ondblclick = function () {
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
};
