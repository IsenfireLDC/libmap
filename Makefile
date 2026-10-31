#### Project Setup ####
PROJ.NAME:=map

PROJ.LIBS:=
PROJ.CUSTOM_MODULES:=
PROJ.FLAGS:=-Wall -Wextra

PROJ.DIR.SRC:=src
PROJ.DIR.INC:=inc
PROJ.DIR.OBJ:=bin
PROJ.DIR.OUT:=.

PROJ.BUILD:=exec
PROJ.UPDATE:=lib header


#### Helper Functions ####
f_findIn=$(shell if [ -d $(1) ]; then find $(1) -type f -iname '$(2)'; fi)
f_prefixFlags=-I$(1)/include -L$(1)/lib


#### Build Files ####
BUILD.FILES.SRC:=$(call f_findIn,$(PROJ.DIR.SRC),*.c)
BUILD.FILES.OBJ:=$(patsubst $(PROJ.DIR.SRC)/%.c,$(PROJ.DIR.OBJ)/%.c.o,$(BUILD.FILES.SRC))
BUILD.FILES.SOBJ:=$(patsubst $(PROJ.DIR.SRC)/%.c,$(PROJ.DIR.OBJ)/%.c.so,$(BUILD.FILES.SRC))
BUILD.FILES.DEP:=$(patsubst $(PROJ.DIR.SRC)/%.c,$(PROJ.DIR.OBJ)/%.c.d,$(BUILD.FILES.SRC))


BUILD.FILES.EXEC:=$(PROJ.DIR.OUT)/$(PROJ.NAME)
BUILD.FILES.SLIB:=$(PROJ.DIR.OUT)/lib$(PROJ.NAME).so
BUILD.FILES.LIB:=$(PROJ.DIR.OUT)/lib$(PROJ.NAME).a


#### Build Flags ####
FLAGS.CUSTOM:=$(call f_prefixFlags,$(CUSTOM_LIBS_DIR)) $(foreach mod,$(PROJ.CUSTOM_MODULES),$(call f_prefixFlags,$(mod)))
FLAGS.LIBS:=$(foreach lib,$(PROJ.LIBS),-l$(lib))
FLAGS.GENERAL:=-I$(PROJ.DIR.INC) $(PROJ.FLAGS) $(FLAGS.CUSTOM)
FLAGS.SHARED:=-shared -fPIC
FLAGS.OUT.LIB:=
FLAGS.OUT.EXEC:=




#### Dynamic/Configurable Targets ####
BUILD.TARGETS.ALL:=\
	$(if $(filter exec,$(PROJ.BUILD)),$(BUILD.FILES.EXEC))\
	$(if $(filter lib,$(PROJ.BUILD)),$(BUILD.FILES.SLIB) $(BUILD.FILES.LIB))

BUILD.TARGETS.UPDATE:=\
	$(if $(filter exec,$(PROJ.UPDATE)),update.exec)\
	$(if $(filter lib,$(PROJ.UPDATE)),update.lib update.slib)\
	$(if $(filter header,$(PROJ.UPDATE)),update.header)




#### Phony Rules ####
all: $(BUILD.TARGETS.ALL)

clean:
	@rm -f $(BUILD.FILES.OBJ) $(BUILD.FILES.DEP) $(BUILD.FILES.SOBJ) $(BUILD.FILES.EXEC) $(BUILD.FILES.SLIB) $(BUILD.FILES.LIB)


-include $(BUILD.FILES.DEP)


#### Single-File Rules ####
$(BUILD.FILES.EXEC): $(BUILD.FILES.OBJ)
	$(info $@ <= $^)
	@gcc -MMD -MP -o $@ $^ $(FLAGS.GENERAL) $(FLAGS.OUT.EXEC) $(FLAGS.LIBS)

$(BUILD.FILES.SLIB): $(filter-out %main.c.so,$(BUILD.FILES.SOBJ))
	$(info $@ <= $^)
	@gcc -MMD -MP -o $@ $^ $(FLAGS.SHARED) $(FLAGS.GENERAL) $(FLAGS.OUT.LIB) $(FLAGS.LIBS)

$(BUILD.FILES.LIB): $(filter-out %main.c.o,$(BUILD.FILES.OBJ))
	$(info $@ <= $^)
	@ar -rcs $@ $^



#### General Rules ####
.SECONDEXPANSION:
$(PROJ.DIR.OBJ)/%.c.o: $(PROJ.DIR.SRC)/%.c | $$(@D)/
	$(info $@ <= $<)
	@gcc -MMD -MP -c -o $@ $< $(FLAGS.GENERAL)

.SECONDEXPANSION:
$(PROJ.DIR.OBJ)/%.c.so: $(PROJ.DIR.SRC)/%.c | $$(@D)/
	$(info $@ <= $<)
	@gcc -MMD -MP -c -o $@ $< $(FLAGS.SHARED) $(FLAGS.GENERAL)


%/:
	@mkdir -p $@




#### Update Rules ####
update: $(BUILD.TARGETS.UPDATE)

update.exec: $(CUSTOM_LIBS_DIR)/bin/$(PROJ.NAME)

update.slib: $(CUSTOM_LIBS_DIR)/lib/lib$(PROJ.NAME).so

update.lib: $(CUSTOM_LIBS_DIR)/lib/lib$(PROJ.NAME).a

update.header: $(patsubst $(PROJ.DIR.INC)/%,$(CUSTOM_LIBS_DIR)/include/%,$(call f_findIn,$(PROJ.DIR.INC),*))

$(CUSTOM_LIBS_DIR)/bin/%: $(PROJ.DIR.OUT)/%
	@cp -vu $< $@

$(CUSTOM_LIBS_DIR)/lib/%.so: $(PROJ.DIR.OUT)/%.so
	@cp -vu $< $@

$(CUSTOM_LIBS_DIR)/lib/%.a: $(PROJ.DIR.OUT)/%.a
	@cp -vu $< $@

.SECONDEXPANSION:
$(CUSTOM_LIBS_DIR)/include/%: $(PROJ.DIR.INC)/% | $$(@D)/
	@cp -vu $< $@



.PRECIOUS: %/
.PHONY: all clean update
