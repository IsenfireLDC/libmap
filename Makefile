# Include project settings file
-include project.mk

# Find files for compilation
FILES.SRC:=$(shell if [ -d $(DIR.SRC) ]; then find $(DIR.SRC) -name '*.c' -type f -print; fi)
FILES.BIN:=$(patsubst $(DIR.SRC)/%.c,$(DIR.BIN)/%.c.o,$(FILES.SRC))
FILES.DEP:=$(FILES.BIN:.o=.d)

# Filter out main from library files
FILES.BIN.LIB:=$(patsubst $(DIR.SRC)/%.c,$(DIR.BIN)/%.c.so,$(filter-out %/main.c,$(FILES.SRC)))
FILES.DEP.LIB:=$(FILES.BIN.LIB:.o=.d)

PROJ.EXEC:=$(DIR.OUT)/$(PROJ.NAME)
PROJ.LIBSH:=$(DIR.OUT)/lib$(PROJ.NAME).so


# Setup flags
FLAGS.LIBS:=$(foreach lib,$(LIBS.SHARED),-l$(lib))
FLAGS:=$(FLAGS.PROJ) -I$(DIR.INC)


# Print file info
ifdef PROJ.SHOWINFO
$(info Source Files: $(FILES.SRC))
$(info Binary Files: $(FILES.BIN))
$(info Dependency Files: $(FILES.DEP))
$(info Executable: $(PROJ.EXEC))
endif


# Begin command rules
all: $(PROJ.EXEC) $(PROJ.LIBSH)

debug: FLAGS:=$(FLAGS) -g3 -ggdb
debug: all

dirs: $(DIR.SRC)/ $(DIR.BIN)/ $(DIR.INC)/

clean:
	@rm -f $(FILES.BIN) $(FILES.DEP) $(PROJ.EXEC) $(PROJ.LIBSH)

update:
	@#cp -vut /home/icenfire/projects/Software/bin $(PROJ.EXEC)
	@cp -vut /home/icenfire/projects/Software/lib $(PROJ.LIBSH)
	@cp -vurt /home/icenfire/projects/Software/include $(DIR.INC)/*


# Include make dependency files
-include $(FILES.DEP)


# Begin generation rules
$(PROJ.EXEC): $(FILES.BIN)
	gcc -MP -MMD -o $@ $^ $(FLAGS) $(FLAGS.EXEC) $(FLAGS.LIBS)

$(PROJ.LIBSH): $(FILES.BIN.LIB)
	gcc -MP -MMD -shared -o $@ $^ $(FLAGS) $(FLAGS.LIBSH) $(FLAGS.LIBS)

# Add the target directory as a prerequisite when building
.SECONDEXPANSION:
$(DIR.BIN)/%.c.o: $(DIR.SRC)/%.c | $$(@D)/
	gcc -MP -MMD -o $@ $< -c $(FLAGS) $(FLAGS.OBJ)

.SECONDEXPANSION:
$(DIR.BIN)/%.c.so: $(DIR.SRC)/%.c | $$(@D)/
	gcc -MP -MMD -shared -o $@ $< -c $(FLAGS) $(FLAGS.OBJ)


%/:
	@mkdir -p "$@"

# Keep generated directories
.PRECIOUS: %/

.PHONY: all dirs clean update
