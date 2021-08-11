python ../esp-idf/components/esptool_py/esptool/esptool.py -p COM3 -b 921600 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size detect --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/app.bin
putty -serial -sercfg 115200 COM3
