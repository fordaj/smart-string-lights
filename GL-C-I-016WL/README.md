# GL-C-I-016WL ESP32 WLED Digital LED Controller

## Identify Hardware

1. Establish UART communication
   1. Plug in with USB-C Data Cable to Windows PC
   1. ID'd CH340 with this command:
      ```powershell
      Get-PnpDevice -PresentOnly | Where-Object {
      >>     $_.Class -eq 'Ports' -or $_.FriendlyName -match 'USB Serial|UART|CH340|CP210|FTDI'
      >> } | Format-Table Status, Class, FriendlyName, InstanceId -AutoSize
      ```
      ```text
      Status Class FriendlyName InstanceId
      ------ ----- ------------ ----------
      Error        USB Serial   USB\VID_1A86&PID_7522\6&2B113EED&0&8
      ```
   1. Error reveals missing drivers; fixed installation of CH340 drivers from a Chinese website: [WCH-IC](https://www.wch-ic.com/downloads/CH341SER_EXE.html)
   1. Re-plugging the board reveals it with powershell command:
      ```powershell
      Get-PnpDevice -PresentOnly | Where-Object {
      >>     $_.InstanceId -like 'USB\VID_1A86&PID_7522*'
      >> } | Format-List Status, FriendlyName, InstanceId
      ```
      ```text
      Status       : OK
      FriendlyName : USB-SERIAL CH340K (COM4)
      InstanceId   : USB\VID_1A86&PID_7522\6&2B113EED&0&8
      ```
1. ID ESP chip with `esptool`:

   ```bash
   uv run esptool --port COM4 chip-id
   ```

   ```text
   esptool v5.3.1
   Connected to ESP32 on COM4:
   Chip type:          ESP32-D0WD-V3 (revision v3.1)
   Features:           Wi-Fi, BT, Dual Core + LP Core, 240MHz, Vref calibration in eFuse, Coding Scheme None
   Crystal frequency:  40MHz
   MAC:                00:4b:12:e9:9d:00

   Stub flasher running.

   Warning: ESP32 has no chip ID. Reading MAC address instead.
   MAC:                00:4b:12:e9:9d:00

   Hard resetting via RTS pin...
   ```

1. ID flash memory:

   ```bash
   uv run esptool --port COM4 flash-id
   ```

   ```text
   esptool v5.3.1
   Connected to ESP32 on COM4:
   Chip type:          ESP32-D0WD-V3 (revision v3.1)
   Features:           Wi-Fi, BT, Dual Core + LP Core, 240MHz, Vref calibration in eFuse, Coding Scheme None
   Crystal frequency:  40MHz
   MAC:                00:4b:12:e9:9d:00

   Stub flasher running.

   Flash Memory Information:
   =========================
   Manufacturer: 5e
   Device: 4016
   Detected flash size: 4MB
   Flash voltage set by a strapping pin: 3.3V

   Hard resetting via RTS pin...
   ```

## Backup Existing Firmware

1. Read in all 4MB flash to a `.bin` file

```bash
uv run esptool --port COM4 read-flash 0 0x400000 factory-backup.bin
```

```text
esptool v5.3.1
Connected to ESP32 on COM4:
Chip type:          ESP32-D0WD-V3 (revision v3.1)
Features:           Wi-Fi, BT, Dual Core + LP Core, 240MHz, Vref calibration in eFuse, Coding Scheme None
Crystal frequency:  40MHz
MAC:                00:4b:12:e9:9d:00

Stub flasher running.

Configuring flash size...
Read 4194304 bytes from 0x00000000 in 387.1 seconds (86.7 kbit/s) to 'factory-backup.bin'.

Hard resetting via RTS pin...
```

1. Verify `.bin` integrity. Expected size: `4194304` bytes

```powershell
Get-Item .\factory-backup.bin | Select-Object Name, Length
Get-FileHash .\factory-backup.bin -Algorithm SHA256
```

```text
Name                Length
----                ------
factory-backup.bin 4194304
```

1. Capture binary hash:

```powershell
Algorithm       Hash                                                                   Path
---------       ----                                                                   ----
SHA256          830748E7ADE220EFCB21F6A1F4803B0B5CDDDF44A58842864243E427A9E52E64       C:\Users\andrewford\Repositories\smart-string-lights\GL-C-I-016WL\factory-backup.bin

```
