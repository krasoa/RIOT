for i in {1,2}
do
	for j in {0..9}
	do
		make all flash -j5 JLINK_SERIAL=$DUT DCDC=0 STDIO_NULL=0 HFCLK_TIMERS=$i CLK_SHIFT=$j
		echo "$i $j"
		measure ~/Nextcloud3/baseline_nrf52dk/hfclk/timers_$i_$j
	done
done
