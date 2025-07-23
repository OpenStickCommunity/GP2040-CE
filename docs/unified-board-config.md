# Unified Board Configuration System

GP2040-CE uses a unified board configuration system where board config directory names directly match Pico SDK board names when possible.

## How It Works

The build system uses `GP2040_BOARDCONFIG` to specify the board configuration directory under `configs/`. For boards that have Pico SDK equivalents, the directory name matches the SDK board name exactly, eliminating the need for mapping.

### Board Configuration Directory Names

| Config Directory | Pico SDK PICO_BOARD | Notes |
|-----------------|---------------------|-------|
| pico | pico | Standard Raspberry Pi Pico |
| pico_w | pico_w | Raspberry Pi Pico W with WiFi |
| pico2 | pico2 | Raspberry Pi Pico 2 |
| pico2_w | pico2_w | Raspberry Pi Pico 2 W with WiFi |
| adafruit_kb2040 | adafruit_kb2040 | Adafruit KB2040 |
| seeed_xiao_rp2040 | seeed_xiao_rp2040 | Seeed XIAO RP2040 |
| sparkfun_promicro | sparkfun_promicro | SparkFun Pro Micro RP2040 |
| sparkfun_promicro_rp2350 | sparkfun_promicro_rp2350 | SparkFun Pro Micro RP2350 |
| waveshare_rp2040_zero | waveshare_rp2040_zero | Waveshare RP2040-Zero |
| FlatboxRev5, ARCController, etc. | pico | Custom boards use pico as SDK fallback |

## Usage

### Setting Board Configuration

You can specify the board configuration in several ways:

1. **Environment variable (recommended):**

   ```bash
   GP2040_BOARDCONFIG=pico_w cmake -B build -S .
   ```

2. **CMake variable:**

   ```bash
   cmake -B build -S . -DGP2040_BOARDCONFIG=pico2_w
   ```

3. **Via VS Code tasks:** Use the predefined tasks for common boards

### Backward Compatibility

The system maintains backward compatibility:
- If `PICO_BOARD` is set without `GP2040_BOARDCONFIG`, it will use the SDK board name directly if a matching config directory exists
- Legacy environment variable usage still works

### Adding New SDK Board Support

To add support for a new SDK board:

1. Create a config directory with the exact SDK board name: `configs/{sdk_board_name}/`
2. Add the board configuration files (`BoardConfig.h`, `CMakeLists.txt`, etc.)
3. The system will automatically detect it's an SDK board and use the directory name as `PICO_BOARD`

### Adding Custom Boards

For custom boards without SDK equivalents:
1. Create a config directory with any descriptive name: `configs/MyCustomBoard/`
2. The system will automatically use `pico` as the SDK fallback
3. Your board config will still be used for GP2040-CE-specific settings

### Benefits

- **No mapping required**: SDK board names are used directly as directory names
- **Automatic detection**: System automatically determines if a board is SDK-supported
- **Simplified maintenance**: No need to maintain mapping functions
- **Better discoverability**: Directory names match SDK documentation
- **Fallback support**: Custom boards automatically get `pico` SDK support

## Migration from Legacy System

Replace:
```bash
PICO_BOARD=pico_w GP2040_BOARDCONFIG=PicoW cmake -B build -S .
```

With:
```bash
GP2040_BOARDCONFIG=pico_w cmake -B build -S .
```

The system will automatically set `PICO_BOARD=pico_w` since the directory name matches the SDK board name.
