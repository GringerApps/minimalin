CONFIG=
CONFIG_VALUE=
ARGS=
MAIL_SUBJECT=""
MAIL_CONTENT=""

.PHONY: test build config_test screenshots screenshot travis clean wipe

config_test:
	@mkdir -p bin
	gcc -std=c99 -g -lcmocka -o bin/config_test -Isrc -Itest src/config.c test/config_test.c -Wl,--wrap=GColorFromHEX -Wl,--wrap=persist_exists -Wl,--wrap=persist_write_data -Wl,--wrap=persist_read_data
	valgrind ./bin/config_test --leak-check=full

screenshots:
	rm -Rf screenshots/
	$(MAKE) screenshot CONFIG=CONFIG_DEFAULT
	$(MAKE) screenshot CONFIG=CONFIG_MILITARY_TIME CONFIG_VALUE=true
	$(MAKE) screenshot CONFIG=CONFIG_DATE_DISPLAYED CONFIG_VALUE=false
	$(MAKE) screenshot CONFIG=CONFIG_WEATHER_ENABLED CONFIG_VALUE=false
	$(MAKE) screenshot CONFIG=CONFIG_RAINBOW_MODE CONFIG_VALUE=true
	$(MAKE) screenshot CONFIG=CONFIG_TEMPERATURE_UNIT CONFIG_VALUE=Fahrenheit
	$(MAKE) screenshot CONFIG=CONFIG_BLUETOOTH_ICON CONFIG_VALUE=NoIcon ARGS='NO_BT=1'
	$(MAKE) screenshot CONFIG=CONFIG_BLUETOOTH_ICON CONFIG_VALUE=Heart ARGS='NO_BT=1'
	pebble kill

screenshot:
	yes | $(CONFIG)=$(CONFIG_VALUE) SCREENSHOT=1 $(ARGS) pebble build
	scripts/screenshots.sh $(CONFIG)_$(CONFIG_VALUE)

build:
	yes | pebble build

travis: build

clean:
	rm -Rf bin
	yes | pebble clean

wipe:
	yes | pebble wipe
