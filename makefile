LIB_NAME = l3d-cube

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
BUILD_DIR = $(SELF_DIR)bin/

FIRMWARE_DIR = ../spark/firmware/
SRC_DIR = firmware/
EXAMPLE_DIR = $(SRC_DIR)examples/

EXAMPLES = $(wildcard $(EXAMPLE_DIR)*.cpp)
EXAMPLE_BINS = $(addprefix $(BUILD_DIR),$(notdir $(patsubst %.cpp,%.bin,$(EXAMPLES))))

SOURCES = firmware/l3d-cube.cpp \
					firmware/neopixel.cpp

export INCLUDE_DIRS = $$(LIB_CORE_LIBRARIES_PATH)$(LIB_NAME) $$(LIB_CORE_LIBRARIES_PATH)$(LIB_NAME)/$(LIB_NAME)

$(EXAMPLE_BINS): $(BUILD_DIR)%.bin : $(EXAMPLE_DIR)%.cpp $(SOURCES) | $(BUILD_DIR)
	$(eval EXAMPLE_NAME=$(notdir $(basename $@)))
	-rm -r $(FIRMWARE_DIR)applications/$(EXAMPLE_NAME)
	cp -r $(EXAMPLE_DIR) $(FIRMWARE_DIR)applications/$(EXAMPLE_NAME)
	cp $(SRC_DIR)*.cpp $(FIRMWARE_DIR)applications/$(EXAMPLE_NAME)
	-rm -r $(FIRMWARE_DIR)libraries/$(LIB_NAME)
	mkdir -p $(FIRMWARE_DIR)libraries/$(LIB_NAME)
	cp -r $(SRC_DIR) $(FIRMWARE_DIR)libraries/$(LIB_NAME)/$(LIB_NAME)
	cd $(FIRMWARE_DIR)build && $(MAKE) APP=$(EXAMPLE_NAME)
	cp $(FIRMWARE_DIR)build/applications/$(EXAMPLE_NAME)/$(EXAMPLE_NAME).bin $@

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

docs:
	doxygen Doxyfile

clean:
	-rm $(EXAMPLE_BINS)
	rmdir $(BUILD_DIR)

.PHONY: docs
