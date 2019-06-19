run:
	python3 server/server.py

test:
	telnet 127.0.0.1 5000 
	JOIN|0|0|0
	MEASUREMENT|1|1|TEMP:20
