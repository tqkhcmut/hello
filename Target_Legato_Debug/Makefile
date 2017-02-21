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
MKCOMMAND:=cd ..;mkapp -t $(TARGET) -o Target_Legato_Debug -w Target_Legato_Debug -i "$(LEGATO_ROOT)/interfaces" -i "$(LEGATO_ROOT)/interfaces/supervisor" -i "$(LEGATO_ROOT)/interfaces/airVantage" -i "$(LEGATO_ROOT)/interfaces/secureStorage" -i "$(LEGATO_ROOT)/interfaces/logDaemon" -i "$(LEGATO_ROOT)/interfaces/atServices" -i "$(LEGATO_ROOT)/interfaces/modemServices" -i "$(LEGATO_ROOT)/interfaces/wifi" -i "$(LEGATO_ROOT)/interfaces/positioning" -s "$(LEGATO_ROOT)/components" -C -g -X -g -L -g hello.adef

# Custom makefile include: definitions
-include ../customDefs.mk

.PHONY: all target clean codegen mkCodegen  airvantage

all: target  airvantage

codegen: mkCodegen

mkCodegen:
	$(MKCOMMAND) --generate-code

target:
	$(MKCOMMAND)

airvantage:
	av-pack -f $(TARGET_ARTIFACT) $(AIRVANTAGE_TYPE)

clean:
	$(CLEANDIR) work app component obj src staging api
	$(CLEAN) *.update *.$(TARGET) manifest.* mktool* *.so *.ninja* .ninja* *.zip

# Custom makefile include: targets
-include ../customTargets.mk
