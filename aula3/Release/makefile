################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
aula3.lss \

FLASH_IMAGE += \
aula3.hex \

EEPROM_IMAGE += \
aula3.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: aula3.elf secondary-outputs

# Tool invocations
aula3.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,aula3.map -mmcu=atmega328p -o "aula3.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

aula3.lss: aula3.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S aula3.elf  >"aula3.lss"
	@echo 'Finished building: $@'
	@echo ' '

aula3.hex: aula3.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex aula3.elf  "aula3.hex"
	@echo 'Finished building: $@'
	@echo ' '

aula3.eep: aula3.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex aula3.elf  "aula3.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: aula3.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega328p aula3.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(EEPROM_IMAGE)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) aula3.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
