# conda activate riot

DUT=682995039
PDU_LENGTHS=(5 15 25 31 100 250 500)

ITVLS=(100 250 500 750 1000 2500 3200)

# STATUS=("slave" "master")
STATUS=("master")

for itvl in "${ITVLS[@]}"; do
    for pdu in "${PDU_LENGTHS[@]}"; do
        for status in "${!STATUS[@]}"; do 
            echo -e "Status: $status (${STATUS[status]})\tPDU: $pdu\t\tITVL: $itvl"
            CFLAGS="-DPDU_SIZE=$pdu -DITVL=$itvl -DCONN_MODE=1 -DNIMBLE_STATCONN_CONN_ITVL_MIN_MS=90U -DNIMBLE_STATCONN_CONN_ITVL_MAX_MS=90U" make all flash -j13  JLINK_SERIAL=$DUT BOARD="nrf52dk"  > /dev/null
            python measure.py --target_status "slave" --itvl "$itvl" --df_name "state_${STATUS[status]}_pdu_${pdu}_itvl_${itvl}.csv"
        done
    done
done
