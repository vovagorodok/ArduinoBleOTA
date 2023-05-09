# Upload tools
Upload using python script:
```
python3 tools/uploader.py <path to bin file>
```
For example:
```
python3 tools/uploader.py .pio/build/esp32dev/firmware.bin
```

Use bash script with automatic bin file finding (works only with VS Code with PlatformIO plugin):
```
./tools/uploader.sh
```
