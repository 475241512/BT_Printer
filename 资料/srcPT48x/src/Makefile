ASM     :=  armasm.exe
CC      :=  armcc.exe
LINK    :=  armlink.exe
OBJCOPY :=  fromelf.exe
RM      := del /q

A_SRC := $(sort $(wildcard *.s))
C_SRC := $(sort $(wildcard *.c))
A_OBJ := $(patsubst %.s,%.o,$(A_SRC))
C_OBJ := $(patsubst %.c,%.o,$(C_SRC))

TARGET := TestDemo_Lv

.PHONY : all clean

# all : $(TARGET).bin $(TARGET).hex
all : $(TARGET).hex

$(TARGET).bin : $(TARGET).axf
	$(OBJCOPY) --bin $< --output $@

$(TARGET).hex : $(TARGET).axf
	$(OBJCOPY) --i32 $< --output $@

$(TARGET).axf : $(A_OBJ) $(C_OBJ) LPC111x.sct
	-$(LINK) --cpu Cortex-M0 --feedback "fb.txt" --strict --scatter "LPC111x.sct" --list "$(TARGET).map" --output "$@" $(A_OBJ) $(C_OBJ)


	$(OBJCOPY) -z $@
	$(OBJCOPY) -z $(C_OBJ)

$(A_OBJ) : %.o : %.s Makefile
	-$(ASM) --cpu Cortex-M0 --thumb --apcs=interwork --depend "$(basename $<).d" -o "$@" $<



$(C_OBJ) : %.o : %.c Makefile
	-$(CC) -c --cpu Cortex-M0 --thumb --apcs=interwork --depend "$(basename $<).d" -O3 --multibyte_chars -I. -I "$(INC)" --feedback "fb.txt" -o "$@" -c $<


clean:
	-$(RM) *.d > nul
	-$(RM) *.o > nul
#-$(RM) fb.txt > nul
	-$(RM) *.map > nul
	-$(RM) *.axf > nul
	-$(RM) $(TARGET).hex > nul
	-$(RM) *.bin > nul

-include $(wildcard *.d)
