#
# AUTO-GENERATED Makefile; do not edit, changes will be lost
#

# Define target type
TARGET:=wp85

# Custom makefile include: initialization
-include ../customInit.mk

# Only when not cleaning
ifneq ($(MAKECMDGOALS),clean)
# Load default PA definitions from main Legato Makefile
$(shell grep 'export PA_DIR .*' $(LEGATO_ROOT)/Makefile > legatoDefs.mk)
$(shell grep 'export .*_PA_.* .*' $(LEGATO_ROOT)/Makefile >> legatoDefs.mk)
-include legatoDefs.mk

# Default targets definitions from Framework
-include targetDefs.$(TARGET)
endif

# Target artifact.*
TARGET_ARTIFACT:=hello.$(TARGET).update

# Airvantage application type
AIRVANTAGE_TYPE?=TMASolution

# Clean commands
CLEAN:=rm -f
CLEANDIR:=rm -Rf

# MK command
MKCOMMAND:=cd ..;mkapp -t $(TARGET) -o /tmp/build/hello/Target_Legato_Debug -w /tmp/build/hello/Target_Legato_Debug -i "$(LEGATO_ROOT)/interfaces/modemServices" -i "$(LEGATO_ROOT)/interfaces/airVantage" -i "$(LEGATO_ROOT)/interfaces/positioning" -i "$(LEGATO_ROOT)/interfaces/atServices" -i "$(LEGATO_ROOT)/interfaces" -i "$(LEGATO_ROOT)/interfaces/wifi" -i "$(LEGATO_ROOT)/interfaces/logDaemon" -i "$(LEGATO_ROOT)/interfaces/secureStorage" -i "$(LEGATO_ROOT)/interfaces/supervisor" -s "." -s "$(LEGATO_ROOT)/components" -C -g -X -g -L -g hello.adef

# Custom makefile include: definitions
-include ../customDefs.mk

.PHONY: all target clean codegen mkCodegen  airvantage sync syncClean

all: target airvantage sync

codegen: mkCodegen sync

mkCodegen:
	$(MKCOMMAND) --generate-code

target:
	$(MKCOMMAND)

airvantage:
	cd /tmp/build/hello/Target_Legato_Debug && \
		av-pack -f $(TARGET_ARTIFACT) $(AIRVANTAGE_TYPE)

sync:
	rsync --copy-links --recursive --checksum --delete --delete-excluded --prune-empty-dirs --filter=". .rsync-rules" /tmp/build/hello/Target_Legato_Debug .. 2>/dev/null || true

clean: syncClean
	$(CLEANDIR) work app component obj src staging api cwe
	$(CLEAN) *.update *.$(TARGET) manifest.* mktool* *.so *.jar *.ninja* .ninja* *.zip

syncClean:
	cd /tmp/build/hello/Target_Legato_Debug; $(CLEANDIR) work app component obj src staging api cwe
	cd /tmp/build/hello/Target_Legato_Debug; $(CLEAN) *.update *.$(TARGET) manifest.* mktool* *.so *.jar *.ninja* .ninja* *.zip

# Custom makefile include: targets
-include ../customTargets.mk
