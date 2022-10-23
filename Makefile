BIN_NAME := thomas
CXX = clang++
SRC_POSTFIX := cpp
SRC_PATH := sauce
BUILD_PATH := build
LIBS = opengl x11 xcursor xi
FLAGS = -Wall -Wpedantic -ldl -pthread
CCFLAGS = ${FLAGS}
LDFLAGS = ${FLAGS}

rwildcard = $(foreach d, $(wildcard $1*), $(call rwildcard,$d/,$2) \
						$(filter $(subst *,%,$2), $d))
CCFLAGS += `pkg-config --libs ${LIBS}`
LDFLAGS += `pkg-config --cflags ${LIBS}`
SOURCES := $(call rwildcard, $(SRC_PATH), *.$(SRC_POSTFIX))
OBJECTS = $(SOURCES:$(SRC_PATH)/%.$(SRC_POSTFIX)=$(BUILD_PATH)/%.o)

${BUILD_PATH}/${BIN_NAME}: ${OBJECTS}
	${CXX} ${CCFLAGS} ${OBJECTS} -o $@

${BUILD_PATH}/%.o: ${SRC_PATH}/%.${SRC_POSTFIX}
	mkdir -p $(@D) 
	${CXX} ${LDFLAGS} -c $< -o $@

.PHONY: all build run clean dirs

all: build

build: ${BUILD_PATH}/${BIN_NAME}

clean:
	rm -rf ${BUILD_PATH}
