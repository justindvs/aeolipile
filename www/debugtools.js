// These are some optional tools used sometimes for debugging or profiling.
function avg(times) {
   var sum = times.reduce(function (a, b) { return a + b; });
   return sum / times.length;
}

function median(times) {
   times.sort(function(a, b) { return a - b; })
   
   var evenLength = (times.length % 2) == 0;
   var middleIdx = Math.floor(times.length / 2);
   
   if (evenLength) {
      var middleTwo = times.slice(middleIdx-1, middleIdx+1);
      return avg(middleTwo);
   }
   else {
      return times[middleIdx];
   }
}

function reportResults(times) {
   var resultStr = '';

   resultStr += '<h1>Results</h1>';
   resultStr += '<pre>';
   resultStr += 'The following are round-trip numbers so the actual latency is roughly half.<br>';
   resultStr += '<br>Num Iterations: ' + times.length;
   resultStr += '<br>First call:     ' + times[0] + ' ms';

   resultStr += '<br><br>(The following numbers exclude the first call)<br>';
   
   var timesExcludingFirst = times.slice(1);
   
   resultStr += '<br>Average time:   ' + avg(timesExcludingFirst) + ' ms';
   resultStr += '<br>Min time:       ' + Math.min.apply(null, timesExcludingFirst) + ' ms';
   resultStr += '<br>Max time:       ' + Math.max.apply(null, timesExcludingFirst) + ' ms';
   resultStr += '<br>Median time:    ' + median(timesExcludingFirst) + ' ms';
   resultStr += '</pre>';

   $('p').html(resultStr);

   // Log the raw results to the console just in case anyone wants to
   // dig into the raw numbers.
   console.log(times);
}

function testLatency(ws, numIterations) {
   var startTime;
   var stopTime;
   var times = new Array();
   var stopCounter = 0;

   // Note: It would be nice to use performance.now() instead of
   // Date.now() to get a higher-resolution timer, but some notable
   // browsers don't support it yet (cough...Safari).
   
   ws.onmessage = function (ev) {
      if (ev.data)
      {
         stopTime = Date.now();
         times.push(stopTime - startTime);
         console.log('WS_STATE: RECV DATA FOR LATENCY TEST: ' + ev.data);
         ++stopCounter;

         if (times.length == numIterations)
         {
            // DONE!
            reportResults(times);
         }
         else
         {
            startTime = Date.now();
            sendCmd(ws, 'ping');
         }
      }
   };
   
   startTime = Date.now();
   sendCmd(ws, 'ping');
}
