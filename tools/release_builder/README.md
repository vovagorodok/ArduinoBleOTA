# Release instructions

1. Change `version` at `library.json` and `library.properties`
2. Change `/<version>/` `at example_hardware_....json` files
3. Run `release_builder.sh`
4. Create version tag/release and attach files from `out` folder
5. Run `pio pkg publish`