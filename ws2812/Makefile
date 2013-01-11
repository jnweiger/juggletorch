# Makefile for project $(NAME)
# Distribute under GPLv2, use with care.
#
# 2013-01-09, jw@suse.de

NAME          = juggletorch_ws2812
CFILES        = ws2812_jw.c

CPU           = tiny861

PROG_HW       = usbtiny		# www.ladyada.com
#PROG_HW       = usbasp		# www.fischl.de
#PROG_HW       = stk200		# www.atmel.com
#PROG_HW       = butterfly	# www.atmel.com
#PROG_HW       = mkbutterfly	# www.mikrokopter.de
#PROG_PORT     = /dev/ttyUSB0
#PROG_PORT     = /dev/rfcomm1
#AVRDUDE_OPT   = -b 57600 -P 

PROG_SW       = avrdude
#PROG_SW       = sudo avrdude	# recommended
#PROG_SW       = uisp		# deprecated

## use this, if $(ISP) magic fails
#UPLOAD_CMD    = $(PROG_SW) -p AT$(CPU) -c $(PROG_HW) $(AVRDUDE_OPT) $(addprefix -P,$(PROG_PORT)) -U $(NAME).hex -v

CFLAGS        += -Os -g -Wall
#LDFLAGS       += -Wl,-u,vfprintf -lprintf_min

TOP_DIR       = .

include $(TOP_DIR)/avr_common.mk
#DIST_EXCLUDE += --exclude photos
CC	= /home/testy/src/svn-co/multiwii/arduino-1.0.3/hardware/tools/avr/bin/avr-g++ -mmcu=at$(CPU)



distclean:: 
	rm -f download* ee_data.* 

## header file dependencies
#############################
include depend.mk
