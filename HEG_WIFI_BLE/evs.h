const char event_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
  body {
    background-color: #707070;
  }
  msgDiv {
    color: white;
  }
  eventDiv {
      color: white;
  }

  input[type=text]{
    border: 2px solid red;
    border-radius: 4px;
    height: 30px;
    padding: 2px;
    font-size: 16px;
  }
  
  .button {
    border: none;
    border-radius: 12px;
    color: white;
    padding: 15px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: 16px;
    margin: 4px 2px;
    cursor: pointer;
   }
   .dattable {
      position: relative;
      width: 75%;
      min-width: 700px;
      table-layout: fixed;
   }
   th {
      color: chartreuse;
      padding: 5px;
      border: 1px solid white;
      width: 10%;
   }
   td {
      color: chartreuse;
      padding: 5px;
      border: 1px solid white;
      width: 10%;
   }
   .scoreth {
      color: honeydew;
   }
   .hegapi {
      position: absolute;
      width: 275px;
      height: 150px;
      top: 125px;
      left: 0%;
   }
   .startbutton {
      background-color: #4CAF50; /* Green */
      position: absolute;
      left: 0%;
   }
   .stopbutton {
      background-color: #FF0000;
      position: absolute;
      left: 120px;
   }
   .sendcommand{
      position: absolute;
      top: 50%;
   }
   .sendbutton{
      background-color: #0000FF; 
   }
   .label { padding: 4px; color: white; }
   .canvascss {
      position: absolute;
      top:125px;
      left:300px;
   }
   .webglcss {
     position: absolute;
     top: 550px;
     left: 10px;
     width: 75%;
     height: 200px;
     min-width: 400px;
   }
   .dummy {
      position: absolute;
      top: 130px;
      left:325px;
   }
</style>
</head>
<body id="main_body">
    <iframe class="dummy" width="0" height="0" border="0" name="dummyframe" id="dummyframe"></iframe>
    <script>
        var ms = [];
        var red = [];
        var ir = [];
        var ratio = [];
        var smallSavLay = [];
        var largeSavLay = [];
        var adcAvg = [];
        var ratioSlope = [0];
        var AI = [];

        var slowSMA = 0;
        var fastSMA = 0;
        var angleChange = 0;
        var scoreArr = [0];

        const VERTEX_LENGTH = 1500;
        var graphY = [...Array(VERTEX_LENGTH).fill(0)];
        
        //appendId is the element Id you want to append this fragment to
        function appendFragment(HTMLtoAppend, appendId) {

            fragment = document.createDocumentFragment();
            var newDiv = document.createElement('div');
            newDiv.innerHTML = HTMLtoAppend;
            newDiv.setAttribute("id", appendId + '_child');

            fragment.appendChild(newDiv);

            document.getElementById(appendId).appendChild(fragment);
        }

        //delete selected fragment. Will delete the most recent fragment if Ids are shared.
        function deleteFragment(parentId,fragmentId) {
            this_fragment = document.getElementById(fragmentId);
            document.getElementById(parentId).removeChild(this_fragment);
        }

        //Separates the appendId from the fragmentId so you can make multiple child threads with different Ids
        function appendFragmentMulti(HTMLtoAppend, appendId, fragmentId) {

            fragment = document.createDocumentFragment();
            var newDiv = document.createElement('div');
            newDiv.innerHTML = HTMLtoAppend;
            newDiv.setAttribute("id", fragmentId + '_child');

            fragment.appendChild(newDiv);

            document.getElementById(appendId).appendChild(fragment);
        }

        var hegapiHTML = '<div id="HEGAPI" class="hegapi"> \
          <form method="post" action="/startHEG" target="dummyframe"><button class="button startbutton" type="submit">Start HEG</button></form> \
          <form method="post" action="/stopHEG" target="dummyframe"><button class="button stopbutton" type="submit">Stop HEG</button></form> \
          <form class="sendcommand" method="post" action="/command" target="dummyframe"><label class="label" for="command">Command:</label><br><input type="text" id="command" name="command"><button class="button sendbutton" type="submit">Send</button></form> \
          </div>';
    
        var dataDivHTML = '<dataDiv id="dataDiv"></dataDiv>'

        var canvasHTML = '<div id="canvasContainer"><canvas class="canvascss" id="myCanvas" height="400px" width="400px"></canvas></div>'

        var shaderHTML = '<div id="shaderContainer"><canvas class="webglcss" id="myShader"></canvas></div>'

        var containerHTML = '<div id="container"></div>';
        var messageHTML = '<msgDiv id="message">Output:</div>';
        var eventHTML = '<eventDiv id="myevent">Not connected...</eventDiv>';
        var tableHeadHTML = '<div id="tableHead"><table class="dattable" id="dataNames"><tr><th>ms</th><th>Red</th><th>IR</th><th>Ratio</th><th>sSavLay</th><th>lSavLay</th><th>adcAvg</th><th>rSlope</th><th>A.I.</th><th class="scoreth">SMA1s-2s</th></tr></table></div>';
        var tableDatHTML = '<div id="tableDat"><table class="dattable" id="dataTable"><tr><th>Awaiting Data...</th></tr></table></div>';

        //Setup page as fragments so updates to elements are asynchronous.
        appendFragment(dataDivHTML,"main_body");
        appendFragment(canvasHTML,"main_body");
        appendFragment(shaderHTML,"main_body");
        appendFragment(hegapiHTML,"main_body");
        appendFragment(containerHTML,"dataDiv");
        appendFragment(messageHTML,"container");
        appendFragment(eventHTML,"container");
        appendFragment(tableHeadHTML,"container");
        appendFragment(tableDatHTML,"container");

        if (!!window.EventSource) {
            var source = new EventSource('/events');

            source.addEventListener('open', function(e) {
                console.log("HEGDUINO", "Events Connected");
                //document.getElementById("message").innerHTML = "Output:";
            }, false);

            source.addEventListener('error', function(e) {
                if (e.target.readyState != EventSource.OPEN) {
                  console.log("HEGDUINO", "Events Disconnected");
                }
            }, false);

            source.addEventListener('message', function(e) {
                document.getElementById("message").innerHTML = e.data;
                console.log("HEGDUINO", e.data);
            }, false);

            source.addEventListener('myevent', function(e) {
                console.log("myevent", e.data);
                if(document.getElementById("myevent").innerHTML != e.data){
                  document.getElementById("myevent").innerHTML = e.data;
                  if(e.data.includes("|")) {
                      var dataArray = e.data.split("|");
                      ms.push(parseInt(dataArray[0]));
                      red.push(parseInt(dataArray[1]));
                      ir.push(parseInt(dataArray[2]));
                      ratio.push(parseFloat(dataArray[3]));
                      smallSavLay.push(parseFloat(dataArray[4]));
                      largeSavLay.push(parseFloat(dataArray[5]));
                      adcAvg.push(parseInt(dataArray[6]));
                      ratioSlope.push(parseFloat(dataArray[7]));
                      AI.push(parseFloat(dataArray[8]));
  
                      if(largeSavLay.length-1 > 20){
                        var temp = largeSavLay.slice(largeSavLay.length - 40,largeSavLay.length);
                        var temp2 = largeSavLay.slice(largeSavLay.length - 20,largeSavLay.length);
                        slowSMA = temp.reduce((a,b) => a + b, 0) / 40;
                        fastSMA = temp2.reduce((a,b) => a + b, 0) / 20;
                        angleChange = fastSMA - slowSMA;
                        scoreArr.push(angleChange);
                        graphY.shift();
                        graphY.push(angleChange);
                        createVertices();
                      }
                      
                      document.getElementById("dataTable").innerHTML = '<tr><td id="ms">'+ms[ms.length-1-1]+'</td><td id="red">'+red[red.length-1-1]+'</td><td id="ir">'+ir[ir.length-1-1]+'</td><td id="ratio">'+ratio[ratio.length-1-1]+'</td><td id="smallSavLay">'+smallSavLay[smallSavLay.length-1-1]+'</td><td id="largeSavLay">'+largeSavLay[largeSavLay.length-1-1]+'</td><td id="adcAvg">'+adcAvg[adcAvg.length-1-1]+'</td><td id="ratioSlope">'+ratioSlope[ratioSlope.length-1-1]+'</td><td id="AI">'+AI[AI.length-1]+'</td><td class="scoreth">'+scoreArr[scoreArr.length-1].toFixed(4)+'</td></tr>'
                  }
                }
            }, false);
        }
      //Based on: https://tinyurl.com/y5roydhe
      let gl,
      shaderProgram,
      vertices,
      canvas;

      const VERTEX_SHADER = `
      attribute vec4 coords;
      attribute float pointSize;
      void main(void) {
        gl_Position = coords;
        gl_PointSize = pointSize;
      }
      `;

      const FRAGMENT_SHADER = `
      precision mediump float;
      uniform vec4 color;
      void main(void) {
        gl_FragColor = color;
      }
      `;

      initGL();
      createShader();
      createVertices();
      draw();
      window.addEventListener('resize', setCanvasSize, false);

      function setCanvasSize() {
          canvas.width = 700;
          canvas.height = 200;
          gl.viewport(0, 0, gl.drawingBufferWidth, gl.drawingBufferHeight);
      }

      function initGL() {
          canvas = document.querySelector('#myShader');
          gl = canvas.getContext('webgl');
          setCanvasSize();
          console.log(gl.drawingBufferWidth, gl.drawingBufferHeight);
          gl.viewport(0, 0, gl.drawingBufferWidth, gl.drawingBufferHeight);
          gl.clearColor(0, 0, 0, 1);
      }
      
      function makePoints(numPoints) {
        const highestPointNdx = numPoints - 1;
        return Array.from({length: numPoints * 2}, (_, i) => {
          const pointId = i / 2 | 0;
          const lerp0To1 = pointId / highestPointNdx;
          const isY = i % 2;
          return isY
            ? graphY[i]           // Y
            : (lerp0To1 * 4 - 1); // X
        });
      }

      function createVertices() {
          vertices = makePoints(VERTEX_LENGTH);
          const buffer = gl.createBuffer();
          gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
          gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.DYNAMIC_DRAW);

          const coords = gl.getAttribLocation(shaderProgram, 'coords');
          gl.vertexAttribPointer(coords, 2, gl.FLOAT, false, 0, 0);
          gl.enableVertexAttribArray(coords);
          // gl.bindBuffer(gl.ARRAY_BUFFER, null);

          const pointSize = gl.getAttribLocation(shaderProgram, 'pointSize');
          gl.vertexAttrib1f(pointSize, 2);

          const uniformColor = gl.getUniformLocation(shaderProgram, 'color');
          gl.uniform4f(uniformColor, normalize(255), normalize(100), normalize(80), 1);
      }

      function createShader() {
          const vs = VERTEX_SHADER;

          const vertexShader = gl.createShader(gl.VERTEX_SHADER);
          gl.shaderSource(vertexShader, vs);
          gl.compileShader(vertexShader);

          const fs = FRAGMENT_SHADER;

          fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
          gl.shaderSource(fragmentShader, fs);
          gl.compileShader(fragmentShader);

          shaderProgram = gl.createProgram();
          gl.attachShader(shaderProgram, vertexShader);
          gl.attachShader(shaderProgram, fragmentShader);

          gl.linkProgram(shaderProgram);
          gl.useProgram(shaderProgram);
      }

      function draw() {
          gl.bufferSubData(gl.ARRAY_BUFFER, 0, new Float32Array(vertices));
          gl.clear(gl.COLOR_BUFFER_BIT);
          gl.drawArrays(gl.LINE_STRIP, 0, VERTEX_LENGTH);
          requestAnimationFrame(draw);
      }

      function normalize(val, max=255, min=0) { return (val - min) / (max - min); }
            

      var mainCanvas = document.getElementById("myCanvas");
      var mainContext = mainCanvas.getContext('2d');
       
      var canvasWidth = mainCanvas.width;
      var canvasHeight = mainCanvas.height;
       
      var angle = 1.57;
       
      var requestAnimationFrame = window.requestAnimationFrame || 
                                  window.mozRequestAnimationFrame || 
                                  window.webkitRequestAnimationFrame || 
                                  window.msRequestAnimationFrame;
       
      function drawCircle() {
          mainContext.clearRect(0, 0, canvasWidth, canvasHeight);
           
          // color in the background
          mainContext.fillStyle = "#2751CE";
          mainContext.fillRect(0, 0, canvasWidth, canvasHeight);
           
          // draw the circle
          mainContext.beginPath();
           
          var radius = 25 + 175 * Math.abs(Math.cos(angle));
          mainContext.arc(200, 200, radius, 0, Math.PI * 2, false);
          mainContext.closePath();
           
          // color in the circle
          mainContext.fillStyle = "#EE1818";
          mainContext.fill();

          if(((angle > 1.57) || (angleChange > 0)) && ((angle < 3.14) || (angleChange < 0))) {
            angle += angleChange*.1;
          }
          requestAnimationFrame(drawCircle);
      }
      drawCircle();
      
  </script>
   
</body>
</html>
)=====";
