# conda activate riot

DUT=682995039
PDU_LENGTHS=(5 15 25 31 100 250 500 1000 1250 1500)

ITVLS=(100 250 500 750 1000 2500 3200)


for itvl in "${ITVLS[@]}"; do
    for pdu in "${PDU_LENGTHS[@]}"; do
        echo -e "PDU: $pdu\t\tITVL $itvl"
        CFLAGS="-DPDU_SIZE=$pdu -DITVL=$itvl" make all flash -j13  JLINK_SERIAL=$DUT BOARD="nrf52dk"  > /dev/null
        python measure.py --df_name "pdu_${pdu}_itvl_${itvl}.csv" --itvl "$itvl"
        # /home/aleks/Nextcloud3/Uni2/Bachelorarbeit/nrf_ppk_api/ppk_cli.py -a 10 -s $PPK -k -o "measurements/pdu_${pdu}_itvl_${itvl}.csv"
	done
done
