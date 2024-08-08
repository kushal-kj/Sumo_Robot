#Check arguments
ifeq ($(HW),LAUNCHPAD)	#HW argument
TARGET_NAME=launchpad
else ifeq ($(MAKECMDGOALS),clean)
else ifeq ($(MAKECMDGOALS),cppcheck)
else ifeq ($(MAKECMDGOALS),format)
#HW argument not required for this rule
else
$(error "Must pass HW=LAUNCHPAD")
endif


#Directories
#TOOLS_PATH = ~/dev/tools
TOOLS_DIR = ${TOOLS_PATH}
MSPGCC_ROOT_DIR = $(TOOLS_DIR)/msp430-gcc
MSPGCC_BIN_DIR = $(MSPGCC_ROOT_DIR)/bin
MSPGCC_INCLUDE_DIR = $(MSPGCC_ROOT_DIR)/include
BUILD_DIR = build/$(TARGET_NAME)
OBJ_DIR = $(BUILD_DIR)/obj

TI_CCS_DIR = $(TOOLS_DIR)/ccs1271/ccs
DEBUG_BIN_DIR = $(TI_CCS_DIR)/ccs_base/DebugServer/bin
DEBUG_DRIVERS_DIR = $(TI_CCS_DIR)/ccs_base/DebugServer/drivers

LIB_DIRS = $(MSPGCC_INCLUDE_DIR)
INCLUDE_DIRS = $(MSPGCC_INCLUDE_DIR) \
	       ./src \
	       ./external/ \
	       ./external/printf



#Toolchain
CC = $(MSPGCC_BIN_DIR)/msp430-elf-gcc
RM=rm
DEBUG = LD_LIBRARY_PATH=$(DEBUG_DRIVERS_DIR) $(DEBUG_BIN_DIR)/mspdebug
CPPCHECK = cppcheck
FORMAT = clang-format-12

#Files
TARGET = $(BUILD_DIR)/$(TARGET_NAME)

SOURCES_WITH_HEADERS = \
					   src/common/assert_handler.c \
					   src/app/drive.c \
					   src/drivers/led.c \
					   src/app/enemy.c \
					   src/drivers/io.c \
					   src/drivers/mcu_init.c \


#SOURCES_WITH_HEADERS = \
#		       src/drivers/uart.c \
#		       src/drivers/i2c.c \
#		       src/app/drive.c \
#		       src/app/enemy.c \


SOURCES = \
	  src/main.c \
	  $(SOURCES_WITH_HEADERS)
	  
HEADERS = \
	  $(SOURCES_WITH_HEADERS:.c=.h) \
	  src/common/defines.h \

OBJECT_NAMES = $(SOURCES:.c=.o)			#naming the .o files same as .c files
OBJECTS = $(patsubst %,$(OBJ_DIR)/%,$(OBJECT_NAMES))		#automatically creating .o files


#Defines
HW_DEFINE = $(addprefix -D,$(HW))	#e.g. -DLAUNCHPAD
DEFINES = $(HW_DEFINE)

#Static Analysis
##Don't check the msp430 helper headers (they have a lot of ifdefs)
CPPCHECK_INCLUDES = ./src
CPPCHECK_IGNORE = external/printf
CPPCHECK_FLAGS = \
		 --quiet --enable=all --error-exitcode=1 \
		 --inline-suppr \
		 --check-config \
		 --suppress=missingIncludeSystem \
		 --suppress=unmatchedSuppression \
		 --suppress=unusedFunction \
		 $(addprefix -I,$(CPPCHECK_INCLUDES)) \
		 $(ADDPREFIX -i,$(CPPCHECK_IGNORE))

#Flags
MCU = msp430f5529
WFLAGS = -Wall -Wextra -Werror -Wshadow					#warning flags
CFLAGS = -mmcu=$(MCU) $(WFLAGS) -fshort-enums $(addprefix -I,$(INCLUDE_DIRS)) $(DEFINES) -Og -g		#compiler flags
LDFLAGS = -mmcu=$(MCU) $(DEFINES) $(addprefix -L,$(LIB_DIRS))			#Linker flags


#Build
##Linking
$(TARGET): $(OBJECTS) $(HEADERS)
	@mkdir -p $(dir $@)		#to check whether the directory is existed ot not 
	$(CC) $(LDFLAGS) $^ -o $@

##Compiling
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^		# $^ = .c files and $@ = .o files



#Phonies

.PHONY: all clean flash cppcheck format

all: $(TARGET)

clean:
	@$(RM) -rf $(BUILD_DIR)

flash: $(TARGET)
	@$(DEBUG) tilib "prog $(TARGET)"

cppcheck:
	@$(CPPCHECK) $(CPPCHECK_FLAGS) $(SOURCES)
format:
	@$(FORMAT) -i $(SOURCES) $(HEADERS)
