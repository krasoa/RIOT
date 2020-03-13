# name of your application
APPLICATION = nimble_beacon

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52dk

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..


# Include NimBLE
USEPKG += nimble
USEMODULE += nimble_svc_gap

# We also use the AD part of the BLE helper module
USEMODULE += bluetil_ad

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

## USAGE: make [blabla] STDIO_NULL=1 DCDC=1 to enable
## Use CFLAGS to integrate into main.c
# Change Modules and Values 

# load stdio_null module
ifdef STDIO_NULL
	USEMODULE += stdio_null
endif

# enable dcdc converter
ifdef DCDC
	CFLAGS+=-DDCDC
endif

# enable number of HFCLK (periph_timer) timers
ifdef HFCLK_TIMERS
	CFLAGS+=-DHFCLK_TIMERS=$(HFCLK_TIMERS)
endif

# How often the base clock of 16MHz shall be divided by 2
ifdef CLK_SHIFT
	CFLAGS+=-DCLK_SHIFT=$(CLK_SHIFT)
endif

# enable LFCLK (rtt)
ifdef RTT
	USEMODULE += periph_rtt
	CFLAGS+=-DRTT
endif

# set advertisement interval manually (default 1000 [=1s])
ifdef ITVL
	CFLAGS+=-DITVL=$(ITVL)
endif

# set payload size manually (default 20 [Byte])
ifdef PAYLOAD_SIZE
	CFLAGS+=-DPAYLOAD_SIZE=$(PAYLOAD_SIZE)
endif

include $(RIOTBASE)/Makefile.include
