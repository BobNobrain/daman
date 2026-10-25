# A macro with rules to compile an executable.
# Accepts executable name and a list of modules that it depends on.
define COMPILE_EXECUTABLE
# Translates executable name into the actual file name to build
$(1): $(OUT_DIR)/$(1)
	@echo "Succesfully compiled $$<"

# Links the object files into the executable
$(OUT_DIR)/$(1): $(OUT_DIR)/$(1).o $(patsubst %,$(OUT_DIR_MODULES)/%.o,$(2))
	@echo "Linking '$$@' with following modules: $(2)..."
	@mkdir -p $$(@D)
	$$(CC) $$^ $$(LD_FLAGS) -o $$@
	@echo

# Creates the object file for the executable
$(OUT_DIR)/$(1).o: src/$(1).c $(foreach module,$(2),src/$(module)/$(module).h)
	@echo "Compiling executable '$1'..."
	@mkdir -p $$(@D)
	$$(CC) $$(CPPFLAGS) $$(CFLAGS) -c $$< -o $$@
	@echo
endef

# A macro with rules to compile a module into a single object file.
# Accepts a module name and a list of module dependencies.
define COMPILE_MODULE
# all module implementation files
$(1)_C_FILES = $$(wildcard src/$(1)/*.c)
# list of object files that the implementations are compiled to
$(1)_O_FILES = $$(patsubst src/$(1)/%.c,$(OUT_DIR_MODULES)/$(1)/%.o,$$($(1)_C_FILES))
# all module declaration files
$(1)_H_FILES = $$(wildcard src/$(1)/*.h)
# all public declarations of module dependencies
$(1)_MODULES = $$(foreach module,$(2),src/$$(module)/$$(module).h)

# Links all object files of the module into a single object file
$(OUT_DIR_MODULES)/$(1).o: $$($(1)_O_FILES)
	@echo "Linking module: '$1'..."
	@echo "  from: $$($(1)_O_FILES)"
	@mkdir -p $$(@D)
	$$(CC) $$^ -r -o $$@
	@echo

# Creates an object file for every .c file of the module
$(OUT_DIR_MODULES)/$(1)/%.o: src/$(1)/%.c $$($(1)_H_FILES) $$($(1)_MODULES) $$(HEADERS)
	@echo "Compiling module: '$1/$$<'..."
	@echo "  with headers: $$($(1)_H_FILES) $$(HEADERS)"
	@mkdir -p $$(@D)
	$$(CC) $$(CPPFLAGS) $$(CFLAGS) -c $$< -o $$@
	@echo
endef
