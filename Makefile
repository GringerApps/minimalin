all: prepare build run clean

prepare:
	@mkdir -p bin

build: config_test

run:
	valgrind ./bin/config_test --leak-check=full

config_test:
	@gcc -std=c99 -g -lcmocka -o bin/config_test -Isrc -Itest src/config.c test/config_test.c -Wl,--wrap=GColorFromHEX -Wl,--wrap=persist_exists -Wl,--wrap=persist_write_data -Wl,--wrap=persist_read_data

clean:
	@rm -R bin
