# Set the windows path you want to move the game inorder to run it (use /mnt/ befor the path so you can access you disk)
WIN_PATH := /mnt/c/Users/Georg/Desktop/

# Set the name of the output
EXEC := threaded

# paths
LIB := 
INCLUDE := ./include
SRC := ./src
BUILD := ./build

#Program args
ARGS := 3

#Default compiler
CC := gcc
CXX := g++

#Debug mode
DEBUG := false

#Exta libs
LIBS := None
EXTRA :=

#For web emcc path
EMCC_ARGS :=
EMCC_PATH := /home/george/Documents/emsdk
EMCC_TEMPLATE := $(LIB)/web/template.html


################################### DO NOT EDIT BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING ###################################


# Makefile options
PLATFORM = LINUX

#compiler options
CFLAGS = -I$(INCLUDE)
CXXFLAGS = -I$(INCLUDE)
LDFLAGS = -lm

SRCS := $(shell find $(SRC) -name '*.c')
OBJS := $(SRCS:$(SRC)/%.c=$(SRC)/%.o)
OBJPATH := $(addprefix $(BUILD)/,$(notdir $(OBJS)))

SRCS_CXX := $(shell find $(SRC) -name '*.cpp')
OBJS_CXX := $(SRCS_CXX:$(SRC)/%.cpp=$(SRC)/%.opp)
OBJPATH_CXX := $(addprefix $(BUILD)/,$(notdir $(OBJS_CXX)))

ifneq ($(PLATFORM),WEB)
	ifeq ($(DEBUG),true)
		CFLAGS += -ggdb -Wall -Werror 
	else
		CFLAGS += -O3 -Werror 
	endif
endif

ifeq ($(PLATFORM),WIN)
	
	ifneq ($(LIBS),None)
		LDFLAGS += -L$(LIB)/windows/ $(LIBS)
	endif

	LDFLAGS += -lgdi32 -lwinmm -lopengl32 -lpthread -lm
	CC = x86_64-w64-mingw32-gcc
	CXX = x86_64-w64-mingw32-g++

	EXEC :=$(EXEC).exe
else ifeq ($(PLATFORM),LINUX)
	ifneq ($(LIBS),None)
		LDFLAGS += -L$(LIB)/linux/ $(LIBS)
	endif

	LDFLAGS += -lGL -lpthread -ldl -lrt -lX11 -lm

	WIN_PATH = ./
else ifeq ($(PLATFORM),WEB)
	CC := emcc
	CXX := em++

	CFLAGS += -DWASM
	LDFLAGS += -L$(LIB)/web $(LIBS) -s ASYNCIFY -s USE_GLFW=3 -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 $(EMCC_ARGS) --shell-file $(EMCC_TEMPLATE)

# SHELL := /bin/bash

	EXEC := $(EXEC).html

	ifneq (,$(wildcard assets))
		LDFLAGS += --preload-file=assets
	endif
	
endif

$(SRC)/%.opp: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ && mv $@ $(BUILD)

$(SRC)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ && mv $@ $(BUILD) 

$(EXEC):$(OBJS) $(OBJS_CXX) $(EXTRA)
	$(CXX) $(OBJPATH) $(OBJPATH_CXX) $(EXTRA) -o $(EXEC) $(LDFLAGS)
	-mv $(EXEC) $(WIN_PATH)

EMCC:
	$(EMCC_PATH)/emsdk activate latest
	source $(EMCC_PATH)/emsdk_env.sh

#Cleaning
PHONY clean:
	rm -f $(BUILD)/*.o $(BUILD)/*.opp $(EXTRA) $(EXEC).exe $(EXEC) $(EXEC).html $(EXEC).js $(EXEC).wasm $(EXEC).data *.js

valgrind:
	$(MAKE) clean
	$(MAKE) $(EXEC) DEBUG=true
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./$(EXEC) $(ARGS)

run: $(EXEC)
	./$(EXEC) $(ARGS)
