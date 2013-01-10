# Makefile for project $(NAME)
# Distribute under GPLv2, use with care.
#
# 2008-09-09, jw

NAME          = juggletorch
OFILES        = $(NAME).o # eeprom.o i2c_slave_cb.o
CPU           = tiny861
TOP_DIR       = .
PROG_HW       = usbasp
PROG_SW       = sudo avrdude

include $(TOP_DIR)/avr_common.mk

distclean:: 
	rm -f download* ee_data.* 

## header file dependencies
#############################
include $(TOP_DIR)/depend.mk
