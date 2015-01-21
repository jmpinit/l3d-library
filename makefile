SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
BUILD_PATH = $(SELF_DIR)obj/

include $(SELF_DIR)tools.mk

SOURCES = firmware/test/stub.cpp \
					firmware/neopixel.cpp \
					firmware/l3d-cube.cpp \
					firmware/examples/blink-an-led.cpp

OBJECTS = $(addprefix $(BUILD_PATH),$(notdir $(SOURCES:.cpp=.o)))

CPPFLAGS += -std=gnu++11 -fno-rtti -fno-exceptions -Werror=deprecated-declarations -Wall -D_TEST -Ifirmware -Ifirmware/test
LDFLAGS =

$(info $$BUILD_PATH is [${BUILD_PATH}])

$(BUILD_PATH)blink-an-led: $(BUILD_PATH) $(OBJECTS)
	$(CPP) $(LDFLAGS) $(OBJECTS) --output $@ 

$(BUILD_PATH)stub.o: firmware/test/stub.cpp
	$(CPP) $(CPPFLAGS) -c $< --output $@

$(BUILD_PATH)neopixel.o: firmware/test/neopixel-stub.cpp
	$(CPP) $(CPPFLAGS) -c $< --output $@

$(BUILD_PATH)l3d-cube.o: firmware/l3d-cube.cpp
	$(CPP) $(CPPFLAGS) -c $< --output $@

$(BUILD_PATH)blink-an-led.o: firmware/examples/blink-an-led.cpp
	$(CPP) $(CPPFLAGS) -c $< --output $@

$(BUILD_PATH):
	mkdir $@

docs:
	doxygen Doxyfile

.PHONY: docs
