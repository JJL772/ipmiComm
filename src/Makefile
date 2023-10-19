# Makefile
TOP = ..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

LIBRARY_IOC += ipmiComm

DBD += ipmiComm.dbd

INCLUDES += $(addprefix -I,$(YAML_INCLUDE))

yaml-cpp_DIR = $(YAML_LIBDIR)

ipmiComm_SRCS += drvMch.c devMch.c drvMchMsg.c ipmiMsg.c ipmiDef.c picmgDef.c
ipmiComm_SRCS += drvMchPicmg.c drvMchServerPc.c
ipmiComm_SRCS += subIpmiComm.c
ipmiComm_SRCS += regConfig.cpp

ipmiComm_DBD += drvMchServerPc.dbd
ipmiComm_DBD += drvMchPicmg.dbd

ipmiComm_LIBS += $(EPICS_BASE_IOC_LIBS) asyn $(YAML_LIB)

ifeq ($(USE_SYSTEM_YAMLCPP),YES)
ipmiComm_SYS_LIBS += yaml-cpp
endif

include $(TOP)/configure/RULES

#----------------------------------------
#  ADD RULES AFTER THIS LINE


