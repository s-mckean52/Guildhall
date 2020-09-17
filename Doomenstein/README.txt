SDST Assignment 01 - Protogame3D

Controls:
	Movement:
		'W' - Move Relative Forward on XY-Plane
		'A' - Move Relative Left on XY-Plane
		'S' - Move Relative Backward on XY-Plane
		'D' - Move Relative Right on XY-Plane
		'Q' - Move Up
		'E' - Move Down

	Other:
		'~' - Open the dev console
		'I' - Set Yaw, Pitch, and Roll to 0
		'O' - Set Position and Rotations to 0
	   'F1' - Debug Draw
	   'F3' - Toggle Possession of Entity

	Netowrking Console Commands:
		startTCPServer port=<portNumber> 	- starts a tcp server on 48000 unless specified otherwise
		stopTCPServer						- stops the current TCP server
		sendMessage message=<message>		- sends message to client or server dependent on how you are connected.
											  Commands do not support spaces
		connect ip=<host:port>				- Connects client to specified ip on a specific port. Defaults to ":48000"
		disconnect							- Disconnects client from server