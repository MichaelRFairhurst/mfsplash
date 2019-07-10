ROOT=/var/lib/mfsplash
BIN_DIR=/usr/local/sbin

bin/mfsplash: src/main/mfsplash.c
	gcc src/main/mfsplash.c -lX11 -lcairo -lXext -lXrandr -o bin/mfsplash -g -lpthread

install: bin/mfsplash
	mkdir ${ROOT}
	mkdir ${ROOT}/writable
	chmod 777 ${ROOT}/writable
	cp -r bin ${ROOT}
	cp -r src/scripts ${ROOT}
	cp -r res/* ${ROOT}
	touch ${ROOT}/lock
	ln -s ${ROOT}/bin/mfsplash ${BIN_DIR}/mfsplash
	ln -s ${ROOT}/scripts/battery.sh ${BIN_DIR}/battery
	ln -s ${ROOT}/scripts/clock.sh ${BIN_DIR}/clock
	ln -s ${ROOT}/scripts/volume.sh ${BIN_DIR}/volume
	ln -s ${ROOT}/scripts/brightness.sh ${BIN_DIR}/brightness
	ln -s ${ROOT}/scripts/batterywatch.sh ${BIN_DIR}/batterywatch
	ln -s ${ROOT}/scripts/playpause.sh ${BIN_DIR}/playpause
	chmod 755 ${BIN_DIR}/mfsplash
	chmod 755 ${BIN_DIR}/battery
	chmod 755 ${BIN_DIR}/clock
	chmod 755 ${BIN_DIR}/volume
	chmod 755 ${BIN_DIR}/brightness
	chmod 755 ${BIN_DIR}/playpause

uninstall:
	rm -rf ${ROOT}
	rm ${BIN_DIR}/mfsplash
	rm ${BIN_DIR}/battery
	rm ${BIN_DIR}/batterywatch
	rm ${BIN_DIR}/clock
	rm ${BIN_DIR}/volume
	rm ${BIN_DIR}/brightness
	rm ${BIN_DIR}/playpause

reinstall:
	make uninstall
	make install
