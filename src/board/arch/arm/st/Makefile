ifeq (,$(findstring gen,$(TYPE)))
    INCLUDE_DIRS += \
    $(addprefix -I,$(shell $(FIND) ./board/arch/$(ARCH)/$(VENDOR)/gen/common -type d -not -path "*Src*")) \
    $(addprefix -I,$(shell $(FIND) ./board/arch/$(ARCH)/$(VENDOR)/gen/$(MCU_FAMILY)/common -type d -not -path "*Src*")) \
    $(addprefix -I,$(shell $(FIND) ./board/arch/$(ARCH)/$(VENDOR)/gen/$(MCU_FAMILY)/$(MCU_BASE)/Drivers -type d -not -path "*Src*"))

    SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/$(VENDOR)/gen/common -regex '.*\.\(s\|c\)')
    SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/$(VENDOR)/gen/$(MCU_FAMILY)/common -regex '.*\.\(s\|c\)')
    SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/$(VENDOR)/gen/$(MCU_FAMILY)/$(MCU_BASE) -regex '.*\.\(s\|c\)')
    SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/$(VENDOR)/variants/$(MCU_FAMILY) -maxdepth 1 -name "*.cpp")
endif