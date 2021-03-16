app:
	$(MAKE) -C ./app

install:
	./rtos/install.sh

format:
	@find ./app -regex ".*\.c" | xargs clang-format -i -style=file --verbose

.PHONY: app format
