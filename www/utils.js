function sendCmd(ws, cmdStr) {
   console.log('WS_CMD: ' + cmdStr);
   ws.send(cmdStr);
}

function preventDoubleEvent(e) {
   // Some mobile browsers send both the touch and click events.  We
   // don't want both to be triggered, so stop event propagation to
   // prevent this.
   e.stopPropagation();
   e.preventDefault();
}

function keyDownFn(ws) {
   return function(e) {
      var thisKey = $(this).data('key');
      sendCmd(ws, 'keydownup ' + thisKey);
      preventDoubleEvent(e);
   };
}

function keySeqFn(ws) {
   return function(e) {
      var sequence = $(this).data('seq');
      sendCmd(ws, 'keyseq ' + sequence);
      preventDoubleEvent(e);
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

   // Setting both touchstart and mousedown event handlers.  This is
   // because I want this to work on both mobile and non-mobile
   // platforms.  If nothing else, it's nice to be able to test on
   // non-mobile platforms with clicking.
   $('[data-key]').touchstart(keyDownFn(ws))
                  .mousedown(keyDownFn(ws));
   $('[data-seq]').touchstart(keySeqFn(ws))
                  .mousedown(keySeqFn(ws));
   
   return ws;
}
