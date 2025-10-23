# Top-level Makefile

# Directories of subprojects
SUBDIRS := client controller vehicle

.PHONY: all clean $(SUBDIRS)

# Default target: build everything
all: $(SUBDIRS)

# Build each subproject
$(SUBDIRS):
	$(MAKE) -C $@

# Clean everything
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done