function sendCmd(ws, cmdStr) {
   console.log('WS_CMD: ' + cmdStr);
   ws.send(cmdStr);
}

function keyDownFn(ws) {
   return function() {
      var thisKey = $(this).data('key');
      sendCmd(ws, 'keydownup ' + thisKey);
   };
}

function keySeqFn(ws) {
   return function() {
      var sequence = $(this).data('seq');
      sendCmd(ws, 'keyseq ' + sequence);
   };
}

function initAeolipile() {
   var ws = new WebSocket('ws://' + location.hostname + ':8080/ws');
   ws.onopen = function () {
      console.log('WS_STATE: open');
   };
   ws.onclose = function () {
      console.log('WS_STATE: closed');
   };
   ws.onerror = function (error) {
      console.log('WS_STATE: error: ' + error);
   };
   ws.onmessage = function (ev) {
      if (!ev.data) {
         console.log('WS_STATE: PING ');
      }
      else {
         console.log('WS_STATE: RECV DATA: ' + ev.data);
      }
   };
   return ws;
}
