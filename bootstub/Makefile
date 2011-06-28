export	PATH	:=	$(DEVKITARM)/bin:$(PATH)

TARGET	:=	bootstub
	
../data/$(TARGET).bin: $(TARGET).elf
	arm-eabi-objcopy -O binary $< $@

$(TARGET).elf: $(TARGET).s Makefile	
	arm-eabi-gcc -Wl,-Ttext=0 -x assembler-with-cpp -nostartfiles -nostdlib $(TARGET).s -o $@


clean:
	rm -f $(TARGET).elf $(TARGET).bin