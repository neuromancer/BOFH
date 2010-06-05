all:
	$(MAKE) -C bme/src
	$(MAKE) -C src

clean: 
	$(MAKE) clean -C bme/src
	$(MAKE) clean -C src
