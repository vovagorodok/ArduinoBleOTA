# Upload tools
## Upload any bin file
Using python script:
```
python3 tools/uploader.py <path to bin file>
```
For example:
```
python3 tools/uploader.py .pio/build/esp32dev/firmware.bin
```

## Upload current bin file
Works only with VS Code with PlatformIO plugin.\
Using python script:
```
python3 tools/current_binary_uploader.py
```
Using bash script:
```
./tools/uploader.sh
```
