# name of your application
APPLICATION = nimble_beacon

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52dk

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..


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
	CFLAGS+=-D_DCDC
endif

# enable number of HFCLK (periph_timer) timers
ifdef HFCLK_TIMERS
	CFLAGS+=-D_HFCLK_TIMERS=$(HFCLK_TIMERS)
	USEMODULE+=periph_timer
endif

# How often the base clock of 16MHz shall be divided by 2
ifdef CLK_SHIFT
ifndef HFCLK_TIMERS
	$(error must specify HFCLK_TIMERS to use CLK_SHIFT)
endif
	CFLAGS+=-D_CLK_SHIFT=$(CLK_SHIFT)
endif

# enable LFCLK (rtt)
ifdef RTT
	USEMODULE += periph_rtt
	CFLAGS+=-D_RTT
endif

# NRF_Timer
ifdef NRF_TIMER
	CFLAGS+=-D_NRF_TIMER=$(NRF_TIMER)
endif

# set advertisement interval manually (default 1000 [=1s])
ifdef ITVL
	CFLAGS+=-D_ITVL=$(ITVL)
endif

# set payload size manually (default 20 [Byte])
ifdef PAYLOAD_SIZE
	$(info hello)
	CFLAGS+=-D_PAYLOAD_SIZE=$(PAYLOAD_SIZE)
endif

ifdef GPIO_MODE
CFLAGS+=-D_GPIO_MODE
ifdef GPIO_OFF
	CFLAGS+=-D_GPIO_OFF
else ifdef GPIO_IN
	CFLAGS+=-D_GPIO_IN
else ifdef GPIO_OUT
	CFLAGS+=-D_GPIO_OUT
endif
endif

ifdef BUTTON_TIMER
	USEMODULE+=periph_gpio_irq
	USEMODULE+=periph_timer
	CFLAGS+=-D_BUTTON_TIMER
endif

include $(RIOTBASE)/Makefile.include
