#/bin/sh

if [ -d temp/ ]; then
	SUB_BUILD=1
else
	SUB_BUILD=0
fi

make SUB=$SUB_BUILD
