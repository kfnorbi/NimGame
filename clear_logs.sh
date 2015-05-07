#/bin/bash

rm log_* &> /dev/null
if [  $? = 0 ]; then
	echo "success"
else
	echo "nothing to delete"
fi