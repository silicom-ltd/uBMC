
OUTPUTCMD="reset;
set terminal "${FORMAT}" size 420,300 medium;
set output '"${OUTPUT}"';
set size square;
set style fill solid 1.0 border -1;
angbase=0;database=0;"

for i in `seq 1 ${ITEMNUM}`; do
	eval data=\${DATA${i}}
	eval color=\${COLOR${i}}
	eval label=\${LABEL${i}}
	OUTPUTCMD=${OUTPUTCMD}"ang"${i}"=((database+"${data}")*360/"${TOTAL}");
set object "${i}" circle at 0,0 size 6 arc [angbase:ang"${i}"] fillcolor rgb '"${color}"' front;
set object 1000+"${i}" rect from 7,2*"${i}" to 8,2*"${i}"+1 fc rgb '"${color}"' fs solid;
set label "${i}" '"${label}"' at 9,2*"${i}"+0.5 left;
angbase=ang"${i}";database=database+"${data}";"
done

OUTPUTCMD=${OUTPUTCMD}"unset border;
unset tics;
unset key;
plot x with lines lc rgb 'white';
set output;"

echo \"${OUTPUTCMD}\" |xargs gnuplot -e 
