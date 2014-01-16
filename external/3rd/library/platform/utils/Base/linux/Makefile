# environment variables:
#	$INCLUDE_PATH is a colon-delimited list of include directories

EMPTY_CHAR=
SPACE_CHAR=$(empty) $(empty)
COLON_CHAR=:

# # # # # # # # # # # # # # # # # # # # #

OUTPUT_FILE_ST=libBase.a
OUTPUT_FILE_MT=libBase_MT.a
OUTPUT_DIR=../../../lib
OBJECT_DIR_ST=./obj_st
OBJECT_DIR_MT=./obj_mt

STL_HOME=../../../../stlport453

INCLUDE_DIRS=.. ../ $(STL_HOME)/stlport

SOURCE_DIRS=. ..
SOURCE_FILES=

LIBRARY_DIRS=
LIBRARY_FILES=

CFLAGS_USER=
LFLAGS_USER=

# # # # # # # # # # # # # # # # # # # # #

INCLUDE_DIR_LIST=$(subst $(COLON_CHAR),$(SPACE_CHAR),$(INCLUDE_PATH)) $(INCLUDE_DIRS) 
INCLUDE_FLAGS=$(addprefix -I,$(INCLUDE_DIR_LIST))
CFLAGS_ST=$(INCLUDE_FLAGS) $(CFLAGS_USER) -D_GNU_SOURCE -Wall -Wno-unknown-pragmas
CFLAGS_MT=$(INCLUDE_FLAGS) $(CFLAGS_USER) -D_GNU_SOURCE -D_REENTRANT -Wall -Wno-unknown-pragmas
CFLAGS_DEBUG_ST=$(CFLAGS_ST) -D_DEBUG -g
CFLAGS_RELEASE_ST=$(CFLAGS_ST) -DNDEBUG -O2
CFLAGS_DEBUG_MT=$(CFLAGS_MT) -D_DEBUG -g
CFLAGS_RELEASE_MT=$(CFLAGS_MT) -DNDEBUG -O2

LIBRARY_FLAGS=$(addprefix -l,$(LIBRARIES))
LFLAGS=$(LIBRARY_FLAGS) $(LFLAGS_USER) -r
LFLAGS_DEBUG=$(LFLAGS)
LFLAGS_RELEASE=$(LFLAGS)

CPP=g++
LINK=ld

# # # # # # # # # # # # # # # # # # # # #

DEBUG_OBJ_DIR_ST=$(OBJECT_DIR_ST)/debug
RELEASE_OBJ_DIR_ST=$(OBJECT_DIR_ST)/release
DEBUG_OBJ_DIR_MT=$(OBJECT_DIR_MT)/debug
RELEASE_OBJ_DIR_MT=$(OBJECT_DIR_MT)/release

DEBUG_DIR=$(OUTPUT_DIR)/debug
RELEASE_DIR=$(OUTPUT_DIR)/release

FIND=$(shell ls $(x)/*.cpp)
SOURCE_LIST = $(SOURCE_FILES) $(foreach x, $(SOURCE_DIRS), $(FIND))

DEBUG_OBJ_LIST_ST=$(addprefix $(DEBUG_OBJ_DIR_ST)/, $(notdir $(SOURCE_LIST:.cpp=.o)))
RELEASE_OBJ_LIST_ST=$(addprefix $(RELEASE_OBJ_DIR_ST)/, $(notdir $(SOURCE_LIST:.cpp=.o)))
DEBUG_OBJ_LIST_MT=$(addprefix $(DEBUG_OBJ_DIR_MT)/, $(notdir $(SOURCE_LIST:.cpp=.o)))
RELEASE_OBJ_LIST_MT=$(addprefix $(RELEASE_OBJ_DIR_MT)/, $(notdir $(SOURCE_LIST:.cpp=.o)))


DEPENDENCY_LIST 	= $(SOURCE_LIST:.cpp=.d)

# # # # # # # # # # # # # # # # # # # # #

release_st: echoRelease_st releaseDirs_st $(RELEASE_DIR)/$(OUTPUT_FILE_ST)
	@echo Successfully built $(RELEASE_DIR)/$(OUTPUT_FILE_ST)
	@echo

debug_st: debugDirs_st $(DEBUG_DIR)/$(OUTPUT_FILE_ST)
	@echo Successfully built $(DEBUG_DIR)/$(OUTPUT_FILE_ST)
	@echo

release_mt: echoRelease_mt releaseDirs_mt $(RELEASE_DIR)/$(OUTPUT_FILE_MT)
	@echo Successfully built $(RELEASE_DIR)/$(OUTPUT_FILE_MT)
	@echo

debug_mt: debugDirs_mt $(DEBUG_DIR)/$(OUTPUT_FILE_MT)
	@echo Successfully built $(DEBUG_DIR)/$(OUTPUT_FILE_MT)
	@echo

all: clean debug_st release_st debug_mt release_mt

all_st: clean debug_st release_st

all_mt: clean debug_mt release_mt

# # # # # # # # # # # # # # # # # # # # #

debugDirs_st :
	@mkdir -p $(DEBUG_OBJ_DIR_ST)
	@mkdir -p $(DEBUG_DIR)

$(DEBUG_OBJ_DIR_ST)/%.o : $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	$(CPP) $(CFLAGS_DEBUG_ST) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))

$(DEBUG_DIR)/$(OUTPUT_FILE_ST): $(DEBUG_OBJ_LIST_ST)
	$(LINK) $(LFLAGS_DEBUG) -o $@ $(DEBUG_OBJ_LIST_ST)


debugDirs_mt :
	@mkdir -p $(DEBUG_OBJ_DIR_MT)
	@mkdir -p $(DEBUG_DIR)

$(DEBUG_OBJ_DIR_MT)/%.o : $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	$(CPP) $(CFLAGS_DEBUG_MT) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))

$(DEBUG_DIR)/$(OUTPUT_FILE_MT): $(DEBUG_OBJ_LIST_MT)
	$(LINK) $(LFLAGS_DEBUG) -o $@ $(DEBUG_OBJ_LIST_MT)


# # # # # # # # # # # # # # # # # # # # #

echoRelease_st :
	@echo Building $(OUTPUT_FILE_ST)
	@echo compiler: $(CPP) $(CFLAGS_RELEASE_ST)
	@echo linker: $(LINK) $(LFLAGS_RELEASE)
	@echo

echoRelease_mt :
	@echo Building $(OUTPUT_FILE_MT)
	@echo compiler: $(CPP) $(CFLAGS_RELEASE_MT)
	@echo linker: $(LINK) $(LFLAGS_RELEASE)
	@echo


# # # # # # # # # # # # # # # # # # # # #

releaseDirs_st :
	@mkdir -p $(RELEASE_OBJ_DIR_ST)
	@mkdir -p $(RELEASE_DIR)

$(RELEASE_OBJ_DIR_ST)/%.o : $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	@echo compiling $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	@$(CPP) $(CFLAGS_RELEASE_ST) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))

$(RELEASE_DIR)/$(OUTPUT_FILE_ST): $(RELEASE_OBJ_LIST_ST)
	@echo linking $(RELEASE_DIR)/$(OUTPUT_FILE_ST)
	@$(LINK) $(LFLAGS_RELEASE) -o $@ $(RELEASE_OBJ_LIST_ST)

releaseDirs_mt :
	@mkdir -p $(RELEASE_OBJ_DIR_MT)
	@mkdir -p $(RELEASE_DIR)

$(RELEASE_OBJ_DIR_MT)/%.o : $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	@echo compiling $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	@$(CPP) $(CFLAGS_RELEASE_MT) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))

$(RELEASE_DIR)/$(OUTPUT_FILE_MT): $(RELEASE_OBJ_LIST_MT)
	@echo linking $(RELEASE_DIR)/$(OUTPUT_FILE_MT)
	@$(LINK) $(LFLAGS_RELEASE) -o $@ $(RELEASE_OBJ_LIST_MT)

# # # # # # # # # # # # # # # # # # # # #

include .depend

.PHONY : $(DEPENDENCY_LIST)

depend: .depend $(DEPENDENCY_LIST)

.depend:
	touch .depend
	make depend

$(DEPENDENCY_LIST):
	@echo Generating dependencies for $(notdir $(@:.d=.cpp))
	@echo -n $(DEBUG_OBJ_DIR_ST)/ >> .depend
	@$(CPP) -MM $(addprefix -I,$(INCLUDE_DIRS)) $(@:.d=.cpp)>> .depend
	@echo -n $(DEBUG_OBJ_DIR_MT)/ >> .depend
	@$(CPP) -MM $(addprefix -I,$(INCLUDE_DIRS)) $(@:.d=.cpp) >> .depend
	@echo -n $(RELEASE_OBJ_DIR_ST)/ >> .depend
	@$(CPP) -MM $(addprefix -I,$(INCLUDE_DIRS)) $(@:.d=.cpp)>> .depend
	@echo -n $(RELEASE_OBJ_DIR_MT)/ >> .depend
	@$(CPP) -MM $(addprefix -I,$(INCLUDE_DIRS)) $(@:.d=.cpp) >> .depend

# # # # # # # # # # # # # # # # # # # # #

clean:
	@echo removing "$(OBJECT_DIR_ST)/*.o"
	@rm -rf $(OBJECT_DIR_ST)
	@echo removing "$(OBJECT_DIR_MT)/*.o"
	@rm -rf $(OBJECT_DIR_MT)
	@echo removing $(DEBUG_DIR)/$(OUTPUT_FILE_ST)
	@rm -f $(DEBUG_DIR)/$(OUTPUT_FILE_ST)
	@echo removing $(RELEASE_DIR)/$(OUTPUT_FILE_ST)
	@rm -f $(RELEASE_DIR)/$(OUTPUT_FILE_ST)
	@echo removing $(DEBUG_DIR)/$(OUTPUT_FILE_MT)
	@rm -f $(DEBUG_DIR)/$(OUTPUT_FILE_MT)
	@echo removing $(RELEASE_DIR)/$(OUTPUT_FILE_MT)
	@rm -f $(RELEASE_DIR)/$(OUTPUT_FILE_MT)
	@rm -f .depend
	@echo


