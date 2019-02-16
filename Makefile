# This is a project Makefile. It is assumed the directory
# this Makefile resides in is a project subdirectory

# Gets the project name from the current foldername
PROJECT_NAME := $(notdir $(CURDIR))

# Esp-idf version supported in project
IDF_VERSION := 3.0.7

# CM_ESP_PATH is overriden by an environment variable to
# the location of esp-idf-x.y.z

# A local copy of https://github.com/espressif/esp-idf
# checked out to the appropriate tag
# git clone -b vx.y.z --recursive https://github.com/espressif/esp-idf
IDF_PATH := $(CM_ESP_PATH)/esp-idf-$(IDF_VERSION)
$(info Using IDF: $(IDF_PATH))

# Toolchain should be kept with idf version
# https://docs.espressif.com/projects/esp-idf/en/vx.y.z/get-started/macos-setup.html
TOOLCHAIN := $(IDF_PATH)/xtensa-esp32-elf

# Put the toolchain binaries in $PATH
export PATH := $(PATH):$(TOOLCHAIN)/bin
export SHELL := env PATH=$(PATH) /bin/bash

# All other Makefile functionality inherited:
include $(IDF_PATH)/make/project.mk
