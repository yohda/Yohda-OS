#/bin/sh

OBJ_DIR=temp
SECTOR_SIZE=512

calc_disk_size() {
	echo "===================== disk image ====================="
	KERN16_SIZE=$(stat -L -c %s temp/bl.bin)
	KERN16_PADD=$(($KERN16_SIZE%$SECTOR_SIZE))
	KERN16_PADD_SIZE=$(($KERN16_SIZE+$SECTOR_SIZE-$KERN16_PADD))

	for (( ; ; ))
	do
			if [ $(($KERN16_PADD_SIZE%$BOCHS_CHS_MAX_SECS)) -ne 0 ]; then
				((KERN16_PADD_SIZE+=$SECTOR_SIZE))	
			else
				break
			fi	
	done

	if [ $(($KERN16_PADD_SIZE / $SECTOR_SIZE)) -ge $BOCHS_CHS_ONE_CYLINDER ]; then
		CHS_HEADS=16
		CHS_SECS=63		
	fi	

	KERN16_PADD=$((($KERN16_PADD_SIZE / $SECTOR_SIZE) % $BOCHS_CHS_ONE_CYLINDER))
	
	KERN16_PADD_SIZE=$((($KERN16_PADD_SIZE / $SECTOR_SIZE) + ($BOCHS_CHS_ONE_CYLINDER - $KERN16_PADD)))  
	CHS_CYLINDES=$(($KERN16_PADD_SIZE /  $BOCHS_CHS_ONE_CYLINDER))
	((KERN16_PADD_SIZE *= $SECTOR_SIZE))
	echo "32-bit kernel size with padded in sector per track(63):" $KERN16_PADD_SIZE
	echo "32-bit kernel secs with padded in sector per track(63):" $(($KERN16_PADD_SIZE/$SECTOR_SIZE))

	echo "===================== reference ====================="
	echo "If you have plan to use bochs, refer to values of CHS in below"
	echo "Sectors per track:" $CHS_SECS	
	echo "heads per cylinder:" $CHS_HEADS
	echo "cylinders:" $CHS_CYLINDES

	cp temp/bl.bin temp/bl.img
	truncate -s $(($KERN16_PADD_SIZE)) temp/bl.img
}

if [ $# -eq 0 ]; then
	echo "error: you must type a build parameter at least"
	exit 1
fi

# In bochs, there are limitations about CHS.  
BOCHS_CHS_MAX_SECS=63
BOCHS_CHS_MAX_HEADS=16
BOCHS_CHS_MAX_CYLINDERS=1024
BOCHS_CHS_ONE_CYLINDER=$(($BOCHS_CHS_MAX_SECS*$BOCHS_CHS_MAX_HEADS))

#if [ $1 == "disk" ]; then
#	if [ ! -d $OBJ_DIR ]; then
#		echo "error: you first must build before making disk image"
#		exit 2
#	fi
if [ $1 == "build" ]; then 
	KERN32_SIZE=$(stat -L -c %s ../p32/temp/kernel.bin)
	echo "32-bit kernel size:" $KERN32_SIZE 
	echo "===================== build ====================="

	KERN32_SECS=$(($KERN32_SIZE/$SECTOR_SIZE))
	make KERN32_SECS=${KERN32_SECS}

	calc_disk_size 
else
	echo "You must type a build parameter at least"
fi
