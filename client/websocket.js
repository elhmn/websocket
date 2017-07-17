``
var socket_onopen = function (event) 
{
	console.log("Socket connection opened !");
	var elem = document.getElementsByTagName("body");
	elem[0].innerHTML += "Socket connection opened !";
};

var socket_onclose = function (event)
{
	var code = event.code;
	var reason = event.reason;
	var wasClean = event.wasClean;

	console.log("Socket connection closed !");
	if (wasClean)
	{
		var elem = document.getElementsByTagName("body");
		console.log(elem[0]);
		elem[0].innerHTML += "Socket connection closed !";
		elem[0].innerHTML += "Connection closed normally !";
	}
	else
	{
		var elem = document.getElementsByTagName("body");
		console.log(elem[0]);
		elem[0].innerHTML +=
							"Connection closed with message : "
							+ reason + "(Code : " + code + ") !";
	}
};

var socket_onerror = function (event)
{
	console.log("Socket Error !");
	var elem = document.getElementsByTagName("body");
	elem[0].innerHTML += "Error : " + event;
}

var socket_onmessage = function (event)
{
	console.log("Socket Message received !");
	var elem = document.getElementsByTagName("body");
	elem[0].innerHTML += "Message received : " + event.data;
}

var websocket_handler = function ()
{
	var uri = "ws://";
	var host_addr = "127.0.0.1";
	var host_port = window.prompt("Enter port number");
	if (isNaN(host_port))
	{
		console.log("Error : wrong port number");
		return ;
	}
	uri = uri + host_addr + ":" + host_port; /* not used for the moment */
	var socket = new WebSocket(uri);
	socket.onopen = socket_onopen;
	socket.onclose = socket_onclose;
	socket.onmessage = socket_onmessage;
	socket.onerror = socket_onerror;
	setInterval(function ()
	{
		if (socket.readyState === WebSocket.OPEN)
		{
			socket.send("Hello gentil animal !");
		}
	}, 1000);
};

var websocket_app = function ()
{
	if (window.WebSocket)
	{
		console.log("Websocket supported !");
		websocket_handler();
	}
	else
	{
		console.log("Websocket not supported !");
		window.alert("Consider update you browser for reacher experience !!");
		//implement a fallback solution
	};
}

window.onload = function ()
{
	websocket_app();
};