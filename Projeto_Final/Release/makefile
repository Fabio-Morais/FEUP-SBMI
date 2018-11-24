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
Projeto_Final.lss \

FLASH_IMAGE += \
Projeto_Final.hex \

EEPROM_IMAGE += \
Projeto_Final.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: Projeto_Final.elf secondary-outputs

# Tool invocations
Projeto_Final.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,Projeto_Final.map -mmcu=atmega328p -o "Projeto_Final.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

Projeto_Final.lss: Projeto_Final.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S Projeto_Final.elf  >"Projeto_Final.lss"
	@echo 'Finished building: $@'
	@echo ' '

Projeto_Final.hex: Projeto_Final.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex Projeto_Final.elf  "Projeto_Final.hex"
	@echo 'Finished building: $@'
	@echo ' '

Projeto_Final.eep: Projeto_Final.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex Projeto_Final.elf  "Projeto_Final.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: Projeto_Final.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega328p Projeto_Final.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(EEPROM_IMAGE)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) Projeto_Final.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
