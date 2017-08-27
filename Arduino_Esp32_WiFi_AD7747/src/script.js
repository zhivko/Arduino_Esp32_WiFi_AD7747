      var currentTimeMs=(new Date()).getTime();
	  var websocket;
	  
	  var output;
	  
	  var preEvtData;

		function searchKeyPress2(e)
		{
		    // look for window.event in case event isn't passed in
		    e = e || window.event;
		    if (e.keyCode == 13)
		    {
		        document.getElementById('btn2').click();
		        return false;
		    }
		    return true;
		}
		
		function handleEnableMotors(cb)
		{
			if(cb.checked)
				doSendCommand("enable");
			else
				doSendCommand("disable");
			
			
		}
		
		function searchKeyPress1(e)
		{
		    // look for window.event in case event isn't passed in
		    e = e || window.event;
		    if (e.keyCode == 13)
		    {
		        document.getElementById('btn1').click();
		        return false;
		    }
		    return true;
		}		

	  function init()
	  {
		output = parent.document.getElementById("output");
		testWebSocket();	
	  }
	  function testWebSocket()
	  {
	  	if (location.host != "")
	  	{
			//var wsUri = "ws://" + location.host + "/";
		}
		else
		{
			var wsUri = "ws://192.168.43.211:81/";
		}
		websocket = new WebSocket(wsUri);
		websocket.onopen = function(evt) { onOpen(evt) };
		websocket.onclose = function(evt) { onClose(evt) };
		websocket.onmessage = function(evt) { onMessage(evt) };
		websocket.onerror = function(evt) { onError(evt) };
	  }
	  function onOpen(evt)
	  {
		writeToScreen("CONNECTED");
		writeToScreen('<span style="color: blue;">Time: ' + ((new Date()).getTime()-currentTimeMs) +'ms Received: ' + evt.data+'</span>');
		//setInterval(function() {
		//	doSend("GiveMeData");
		//}, 150);	
	  }
	  function onClose(evt)
	  {
		writeToScreen("DISCONNECTED");
	  }
	  function onMessage(evt)
	  {
		  
		if(evt.data != preEvtData)
		{		  
			writeToScreen('<span style="color: blue;">Time: ' + ((new Date()).getTime()-currentTimeMs) +'ms Received: ' + evt.data+'</span>');
			var res = evt.data.split(' ');
			if (evt.data.slice(0,1) == 'X' && res.length==5)
			{
				document.getElementById("roseSvg").contentDocument.getElementById('PositionX').textContent = res[0];
				document.getElementById("roseSvg").contentDocument.getElementById('PositionY').textContent = res[1];
				document.getElementById("roseSvg").contentDocument.getElementById('PositionZ').textContent = res[2];
				document.getElementById("roseSvg").contentDocument.getElementById('PositionE').textContent = res[3];
				if(endsWith(res[4],"1"))
					document.getElementById("enableMotors").checked="checked";
				else
					document.getElementById("enableMotors").checked="";
			}
			else if(evt.data.slice(0,8) == 'Analogue')
			{
				document.getElementById('Analogue').textContent = res[1];
			}
			else if(evt.data.slice(0,7) == 'Encoder')
			{
				document.getElementById('Encoder').textContent = res[1];
			}
		}
		preEvtData = evt.data; 
	  }
	  function onError(evt)
	  {
		writeToScreen('<span style="color: red;">ERROR:</span> ' + evt.data);
	  }
	  function doSend(element)
	  {
		//writeToScreen("SENT: " + message); 
		textToSend = element.value;
		websocket.send(textToSend);
	  }
	  function doSendCommand(textToSend)
	  {
		if(websocket!=null)
			websocket.send(textToSend);
	  }
	  function writeToScreen(message)
	  {
		// var pre = parent.document.createElement("p");
		// pre.style.wordWrap = "break-word";
		// pre.innerHTML = message;
		// output.appendChild(pre);
		
		output.innerHTML = message + "<br>\n" + output.innerHTML;		
	  }
	  function doDisconnect()
	  {
		var disconnect = document.getElementById("disconnect");
		disconnect.disabled = true;
		websocket.close();
	  }

	function jogXYClick(a)
	{

		var radios = document.getElementsByName('translateOrOposite');
		var translate='';
		for (var i = 0, length = radios.length; i < length; i++) {
		    if (radios[i].checked) {
		        translate = radios[i].value;
		        break;
		    }
		}
		var coord = a.substring(0, 1);
		var sign = a.substring(1, 2);
		var value = a.substring(2, a.length) * 10;
		if (translate=='translate')
		{
			if (sign=='+')
				signOposite='-';
			else
				signOposite='+';
			if (coord == 'X' || coord == 'Y')
				var command = "X" + sign + value + " Y" + signOposite + value;
			else
				var command = "Z" + sign + value + " E" + signOposite + value;
		}
		else
		{
			if (coord == 'X' || coord == 'Y')
				if(sign=="-")
					var command = "X" + sign + value + " Y" + sign + value;
				else
					var command = "Y" + sign + value + " X" + sign + value;
			else
				if(sign=="-")
					var command = "E" + sign + value + " Z" + sign + value;
				else
					var command = "Z" + sign + value + " E" + sign + value;				
		}
		writeToScreen("SENT: " + command); 
		doSendCommand(command);
	}

	function jogClick(a)
	{
		writeToScreen("SENT: " + a); 
		doSendCommand(a);		
	}

	function endsWith(str, suffix) {
	    return str.indexOf(suffix, str.length - suffix.length) !== -1;
	}

window.addEventListener("load", init, false);