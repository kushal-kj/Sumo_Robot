#Directories
TOOLS_DIR = ${TOOLS_PATH}
MSPGCC_ROOT_DIR = $(TOOLS_DIR)/msp430-gcc
MSPGCC_BIN_DIR = $(MSPGCC_ROOT_DIR)/bin
MSPGCC_INCLUDE_DIR = $(MSPGCC_ROOT_DIR)/include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
TI_CCS_DIR = $(TOOLS_DIR)/ccs1271/ccs
DEBUG_BIN_DIR = $(TI_CCS_DIR)/ccs_base/DebugServer/bin
DEBUG_DRIVERS_DIR = $(TI_CCS_DIR)/ccs_base/DebugServer/drivers

LIB_DIRS = $(MSPGCC_INCLUDE_DIR
INCLUDE_DIRS = $(MSPGCC_INCLUDE_DIR) \
	       ./src \
	       ./external/ \
	       ./external/printf



#Toolchain
CC = $(MSPGCC_BIN_DIR)/msp430-elf-gcc
RM=rm
DEBUG = LD_LIBRARY_PATH=$(DEBUG_DRIVERS_DIR) $(DEBUG_BIN_DIR)/mspdebug
CPPCHECK = cppcheck

#Files
TARGET = $(BIN_DIR)/sumo_robot

SOURCES = main.c \
	  
OBJECT_NAMES = $(SOURCES:.c=.o)			#naming the .o files same as .c files
OBJECTS = $(patsubst %,$(OBJ_DIR)/%,$(OBJECT_NAMES))		#automatically creating .o files


#Flags
MCU = msp430f5529
WFLAGS = -Wall -Wextra -Werror -Wshadow					#warning flags
CFLAGS = -mmcu=$(MCU) $(WFLAGS) $(addprefix -I,$(INCLUDE_DIRS)) -Og -g		#compiler flags
LDFLAGS = -mmcu=$(MCU) $(addprefix -L,$(LIB_DIRS))			#Linker flags


#Build
##Linking
$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)		#to check whether the directory is existed ot not 
	$(CC) $(LDFLAGS) $^ -o $@

##Compiling
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^		# $^ = .c files and $@ = .o files



#Phonies

.PHONY: all clean flash cppcheck

all: $(TARGET)

clean:
	$(RM) -r $(BUILD_DIR)

flash: $(TARGET)
	$(DEBUG) tilib "prog $(TARGET)"

cppcheck:
	@$(CPPCHECK) --quiet --enable=all --error-exitcode=1 \
		--inline-suppr \
		-I (INCLUDE_DIRS) \
		$(SOURCES) \
		-i external/printf