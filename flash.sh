python ./rtos/components/esptool_py/esptool/esptool.py --chip esp8266 --port COM4 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB 0x0 ./app/build/bootloader/bootloader.bin 0x10000 ./app/build/main.bin 0x8000 ./app/build/partitions_singleapp.bin
putty -serial -sercfg 115200 COM4
