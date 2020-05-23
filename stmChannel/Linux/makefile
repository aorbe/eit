HEX_NAME = stmChannel
CC  = arm-none-eabi-gcc
AS  = arm-none-eabi-as
LD  = arm-none-eabi-ld -v
CP  = arm-none-eabi-objcopy
LDSCRIPT = stm32_flash.ld

FW  := /home/alsantos/git/eit_code/STM32F4-Discovery
INC_DISCOVERY := $(FW)/Utilities/STM32F4-Discovery
SRC_DISCOVERY := $(FW)/Utilities/STM32F4-Discovery
INC_STD_DRIVER:=$(FW)/Libraries/STM32F4xx_StdPeriph_Driver/inc
SRC_STD_DRIVER:=$(FW)/Libraries/STM32F4xx_StdPeriph_Driver/src
INC_USB_OTG := $(FW)/Libraries/STM32_USB_OTG_Driver/inc
SRC_USB_OTG := $(FW)/Libraries/STM32_USB_OTG_Driver/src
INC_USB_DEV := $(FW)/Libraries/STM32_USB_Device_Library/Core/inc
SRC_USB_DEV := $(FW)/Libraries/STM32_USB_Device_Library/Core/src
INC_USB_HID := $(FW)/Libraries/STM32_USB_Device_Library/Class/hid/inc
SRC_USB_HID := $(FW)/Libraries/STM32_USB_Device_Library/Class/hid/src
SRC_FAST_M  := $(FW)/Libraries/CMSIS/DSP_Lib/Source/FastMathFunctions
INC_FAST_M  := $(FW)/Libraries/CMSIS/Include

EXTRA_INCLUDE = -I$(FW)/Project -I$(FW)/Libraries/CMSIS/ST/STM32F4xx/Include -I$(FW)/Libraries/CMSIS/Include
INCLUDE = $(EXTRA_INCLUDE) -I$(INC_DISCOVERY) -I$(INC_STD_DRIVER) -I$(INC_USB_DEV)  -I$(INC_USB_OTG) -I$(INC_USB_HID) -I$(INC_FAST_M) -I../inc 

DEFS += -DUSE_STDPERIPH_DRIVER -DSTM32F4XX -D__FPU_PRESENT="1" -DMANGUSTA_DISCOVERY -DARM_MATH_CM4 -DHSE_VALUE=8000000 $(NODE)
LIBS = -lm -lc
MC_FLAGS = -Ofast -mcpu=cortex-m4 -mtune=cortex-m4 -mthumb -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fsingle-precision-constant -nostartfiles  
AS_FLAGS = 
C_FLAGS = -fverbose-asm -c $(MC_FLAGS) $(INCLUDE) $(LIBS) $(DEFS) -Wl,-T,../stm32_flash.ld -MD -MP -O3
LD_FLAGS = $(MC_FLAGS) $(INCLUDE) $(DEFS) -Wl,-T,../stm32_flash.ld -MD -MP -Os

user_objects = $(patsubst %c, %o, $(wildcard ../src/*.c))
SRC_SYSTEM = 
#$(FW)\Libraries\CMSIS\ST\STM32F4xx\Source\Templates\arm\startup_stm32f4xx.s
#
C_DISCOVERY := stm32f4_discovery.c
# stm32f4_discovery_audio_codec.c stm32f4_discovery_lis302dl.c 
C_STD_DRIVER :=  misc.c stm32f4xx_gpio.c stm32f4xx_rcc.c stm32f4xx_exti.c stm32f4xx_syscfg.c stm32f4xx_tim.c stm32f4xx_usart.c stm32f4xx_dma.c
#  stm32f4xx_flash.c stm32f4xx_adc.c stm32f4xx_i2c.c stm32f4xx_dac.c stm32f4xx_spi.c
C_USB_OTG := usb_dcd_int.c usb_core.c usb_dcd.c
C_USB_DEV := usbd_core.c usbd_req.c usbd_ioreq.c
C_USB_HID := usbd_hid_core.c
C_FAST_M  := arm_cos_f32.c arm_sin_f32.c
# arm_sqrt_f32.c 
 
OBJ_DISCOVERY := $(patsubst %.c, %.o, $(notdir $(C_DISCOVERY)))
$(OBJ_DISCOVERY) : %.o : $(SRC_DISCOVERY)/%.c
	@echo CREATING $@ FROM $<
	@$(CC) $(C_FLAGS) $< -o $@

OBJ_STD_DRIVER := $(patsubst %.c, %.o, $(notdir $(C_STD_DRIVER)))
$(OBJ_STD_DRIVER) : %.o : $(SRC_STD_DRIVER)/%.c
	@echo CREATING $@ FROM $<
	@$(CC) $(C_FLAGS) $< -o $@

OBJ_USB_OTG := $(patsubst %.c, %.o, $(notdir $(C_USB_OTG)))
$(OBJ_USB_OTG) : %.o : $(SRC_USB_OTG)/%.c
	@echo CREATING $@ FROM $<
	@$(CC) $(C_FLAGS) $< -o $@

OBJ_USB_DEV := $(patsubst %.c, %.o, $(notdir $(C_USB_DEV)))
$(OBJ_USB_DEV) : %.o : $(SRC_USB_DEV)/%.c
	@echo CREATING $@ FROM $<
	@$(CC) $(C_FLAGS) $< -o $@

OBJ_USB_HID := $(patsubst %.c, %.o, $(notdir $(C_USB_HID)))
$(OBJ_USB_HID) : %.o : $(SRC_USB_HID)/%.c
	@echo CREATING $@ FROM $<
	@$(CC) $(C_FLAGS) $< -o $@

OBJ_FAST_M := $(patsubst %.c, %.o, $(notdir $(C_FAST_M)))
$(OBJ_FAST_M) : %.o : $(SRC_FAST_M)/%.c
	@echo CREATING $@ FROM $<
	@$(CC) $(C_FLAGS) $< -o $@

$(user_objects) : %.o : %.c
	@echo CREATING $@ FROM $<
	@$(CC) $(C_FLAGS) ../src/$< -o $@

objects := $(user_objects) $(OBJ_STD_DRIVER) $(OBJ_FAST_M)

startup.o : $(FW)/Libraries/CMSIS/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f4xx.s
	@echo CREATING $@ FROM $<
	@$(AS) $(AS_FLAGS) $< -o $@
	
$(HEX_NAME).elf: $(objects) startup.o
	@echo LINKING...
	@$(CC) $(LD_FLAGS) $(objects) startup.o -o $@ $(LIBS) 
	
all: $(HEX_NAME).elf
	$(CP) -O binary $^ $(HEX_NAME).bin
	
.PHONY:	clean

clean:
	rm *.mk *.lst ../src/*.o ../src/*.d *.o *.d FW_STM32 src -r
