#!/bin/bash

PDU_LENGTHS=(3 8 15 20 25 30 31)
ITVLS=(100 250 500 750 1000 1250 2500 3750 5000)

for pdu in "${PDU_LENGTHS[@]}"; do
	for itvl in "${ITVLS[@]}"; do
		echo $pdu $itvl
		CFLAGS="-DITVL=$itvl -DPDU_LENGTH=$pdu" make all flash -j13  JLINK_SERIAL=$DUT BOARD="nrf52dk" > /dev/null
		/home/aleks/Nextcloud3/Uni2/Bachelorarbeit/nrf_ppk_api/ppk_cli.py -a 120 -p 2 -s $PPK -k -o "measurements/pdu_${pdu}_itvl_${itvl}.csv"
	done
done
