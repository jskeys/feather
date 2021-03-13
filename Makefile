app:
	$(MAKE) -C ./app

install:
	./rtos/install.sh

.PHONY: app
