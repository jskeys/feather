#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := main
GIT_VERSION := $(shell git describe --abbrev=7 --dirty --always --tags)
CFLAGS := -DAPP_HASH=\"$(GIT_VERSION)\"
EXTRA_COMPONENT_DIRS := $(shell pwd)/../libs/

include $(IDF_PATH)/make/project.mk
