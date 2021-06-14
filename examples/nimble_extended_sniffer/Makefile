# name of your application
APPLICATION = nimble_extended_sniffer

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52dk

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

USEMODULE += bluetil_ad
CFLAGS += -DMYNEWT_VAL_MSYS_1_BLOCK_SIZE=294
CFLAGS += -DMYNEWT_VAL_MSYS_1_BLOCK_COUNT=32

# Activate BLE extended advertisings
CFLAGS += -DMYNEWT_VAL_BLE_EXT_ADV=1
CFLAGS += -DMYNEWT_VAL_BLE_LL_CFG_FEAT_LL_EXT_ADV=1

# ~maximale Größe der BLE GAP events (nicht höher als 257 gehen!)
CFLAGS += -DMYNEWT_VAL_BLE_HCI_EVT_BUF_SIZE=257
CFLAGS += -DMYNEWT_VAL_BLE_HCI_EVT_HI_BUF_COUNT=12

# Wie vielen Paketen maximal auf den Datenkanälen gefolgt werden kann
CFLAGS += -DMYNEWT_VAL_BLE_LL_EXT_ADV_AUX_PTR_CNT=10

# Maximale extended advertising size. Max ist 1650
CFLAGS += -DMYNEWT_VAL_BLE_EXT_ADV_MAX_SIZE=1650

# Weitere radio modi, 0 -> aus 1 -> an (wer hätte es gedacht)
CFLAGS += -DMYNEWT_VAL_BLE_LL_CFG_FEAT_LE_2M_PHY=0
CFLAGS += -DMYNEWT_VAL_BLE_LL_CFG_FEAT_LE_CODED_PHY=0

# Anzahl Advertising Instanzen. Das ist immer {VARIABLE}+1
# -> Variable = 0 = 1 Advertising-Instanz
CFLAGS += -DMYNEWT_VAL_BLE_MULTI_ADV_INSTANCES=0

# Include NimBLE
USEPKG += nimble
# USEMODULE += nimble_svc_gap
# USEMODULE += nimble_svc_gatt

# We also use the AD part of the BLE helper module

# Disable STDIO
# USEMODULE += stdio_null



# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
