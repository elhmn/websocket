###Simple websocket Server###

Install first setup wslay :

			Dependancies :
							cunit >= 2.1
							nettle >= 2.4
							python-sphinx for man generation

			Setup nettle-3.3:
							./install_nettle
			
			Caution --> it is possible that the command make install end unexpectedly
					--> Don't worry you will still be able to run the program using 
					--> The command :
					-->					make static

			Setup wslay :
							./install_wslay

Compile :
			make
		or
			make static

Run :
			./server


Then run the index.html client and provide him with the sever port
he must listen to
