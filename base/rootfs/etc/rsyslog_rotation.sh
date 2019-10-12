LOG_FILE_PREFIX=/var/log/messages
ROTATE_NUM=8

if [ "$1" != "" ]; then
	LOG_FILE_PREFIX=$1
fi
if [ "$2" != "" ]; then 
	ROTATE_NUM=$2   
fi

for i in `seq 1 $(($ROTATE_NUM-1))`; do
	CURRID=$(($ROTATE_NUM-$i))
	NEXTID=$(($CURRID+1))
	if [ -e $LOG_FILE_PREFIX.$CURRID ]; then
		mv $LOG_FILE_PREFIX.$CURRID $LOG_FILE_PREFIX.$NEXTID
	fi
done

mv -f $LOG_FILE_PREFIX $LOG_FILE_PREFIX.1
touch $LOG_FILE_PREFIX

