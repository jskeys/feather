app:
	idf.py all -C ./server

install:
	./rtos/install.sh

format:
	@find ./server/main -regex ".*\.[ch]p*" | xargs clang-format -i -style=file --verbose

.PHONY: app format
