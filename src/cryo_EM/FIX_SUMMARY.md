# Fix Summary - Exit Code 139 Segmentation Fault

## ✅ Problem Resolved

The program can now successfully load large MRC files (1024x300x1024, 300MB) without crashing!

## Issues Discovered

### 1. **No Error Checking for File Operations** (lib/load.hpp)
- `fopen()` failure not checked
- `fread()` return value not verified
- Memory allocation failure not caught

### 2. **Signal-Slot Timing Issue** (mainwindow.cpp)
- `setCenterZ()` triggers signal → `spinBox_z->setValue()` → `on_spinBox_z_valueChanged()`
- Attempted to access image before data was fully loaded, causing segmentation fault

### 3. **Array Indexing Error** (lib/load.hpp)
- `data` array structure is `data[z][y][x]`
- But index order was wrong when accessing in `clip_slow()`
- Example: `data[tz][j][i]` should be `data[tz][i][j]`

### 4. **Invalid uint32_t Boundary Check**
```cpp
uint32_t tx = (d - b * i - c * j) / a + 0.5;
if (tx < 0 || tx >= shape[2]) {  // tx can never be < 0!
```

## Fixes Applied

### Fix 1: lib/load.hpp - Add Complete Error Handling

```cpp
bool load(const string& file_path) {
    // ✓ Check file open
    FILE* file = fopen(file_path.c_str(), "rb");
    if (file == nullptr) {
        fprintf(stderr, "Error: Cannot open file: %s\n", file_path.c_str());
        return false;
    }

    // ✓ Verify shape read
    size_t read_count = fread(&(shape[0]), sizeof(uint32_t), 3, file);
    if (read_count != 3) {
        fprintf(stderr, "Error: Failed to read shape\n");
        fclose(file);
        return false;
    }

    // ✓ Check shape validity
    if (shape[0] == 0 || shape[1] == 0 || shape[2] == 0) {
        fprintf(stderr, "Error: Invalid shape\n");
        fclose(file);
        return false;
    }

    // ✓ Check data size limit (4GB)
    size_t total_size = (size_t)shape[0] * shape[1] * shape[2];
    const size_t MAX_SIZE = 4ULL * 1024 * 1024 * 1024;
    if (total_size > MAX_SIZE) {
        fprintf(stderr, "Error: Data too large\n");
        fclose(file);
        return false;
    }

    // ✓ Catch memory allocation exception
    try {
        array = new uint8_t[total_size];
    } catch (const std::bad_alloc& e) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return false;
    }

    // ✓ Verify data read
    read_count = fread(array, sizeof(uint8_t), total_size, file);
    if (read_count != total_size) {
        fprintf(stderr, "Error: Failed to read data\n");
        delete[] array;
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}
```

### Fix 2: mainwindow.cpp - Block Signals to Avoid Premature Triggering

```cpp
// Block signals while loading data
{
    QSignalBlocker blocker1(ui->spinBox_x);
    QSignalBlocker blocker2(ui->spinBox_y);
    QSignalBlocker blocker3(ui->spinBox_z);
    QSignalBlocker blocker4(ui->horizontalSlider_x);
    QSignalBlocker blocker5(ui->horizontalSlider_y);
    QSignalBlocker blocker6(ui->horizontalSlider_z);

    setCenterX(shape[2] >> 1);
    setCenterY(shape[1] >> 1);
    setCenterZ(shape[0] >> 1);
}
// Signal blockers automatically restore signals when leaving scope
```

### Fix 3: lib/load.hpp - Correct Array Index Order

```cpp
// XY plane slice (z-direction projection)
for (uint32_t i = 0; i < height; ++i) {      // i traverses y
    uint8_t* line = image.scanLine(i);
    for (uint32_t j = 0; j < width; ++j) {   // j traverses x
        double tz_calc = (d - a * i - b * j) / c + 0.5;
        if (tz_calc < 0 || tz_calc >= shape[0]) {
            line[j] = 0;
        } else {
            uint32_t tz = (uint32_t)tz_calc;
            line[j] = data[tz][i][j];  // Fixed: data[z][y][x]
        }
    }
}
```

### Fix 4: Correct Boundary Check

Changed `uint32_t` calculation to first compute with `double`, then check boundaries, then convert:

```cpp
double tz_calc = (d - a * i - b * j) / c + 0.5;
if (tz_calc < 0 || tz_calc >= shape[0]) {  // Now can correctly check negative values
    line[j] = 0;
} else {
    uint32_t tz = (uint32_t)tz_calc;
    line[j] = data[tz][i][j];
}
```

## Test Results

### Successfully Loaded Large Data
```
Loading data with shape [1024, 300, 1024], total 314572800 bytes (300.00 MB)
Data loaded successfully
Message:show binary file
Message:About to call get_shape()
Message:get_shape() returned, shape[0]= 1024 shape[1]= 300 shape[2]= 1024
Message:Shape is valid, about to setMaxCenter...
Message:setMaxCenter done, about to setCenter...
Message:setCenter done, about to call getXYImage()
Message:getXYImage done, about to call drawXYImage()
Message:drawXYImage done, about to set OpenGL widget...
Message:OpenGL widget set, about to click buttons...
Message:pushButton_2d clicked
Message:pushButton_xy_only clicked
Message:All done successfully!
```

**✅ No crashes, all steps completed successfully!**

## About Python Packages

Tests show that `ncempy` is correctly installed, no missing package issues. If running in other environments, need to install:

```bash
pip install ncempy
```

## About MRC File Modification

**The MRC file itself has not been modified**. The program workflow is:
1. Read MRC file (read-only)
2. Convert to .binaryData format (new file)
3. Subsequently load .binaryData file directly

The MRC file remains unchanged, only a cached .binaryData file is generated for fast loading.

## File List

Modified files:
- `lib/load.hpp` - Added error handling, corrected array indexing
- `mainwindow.cpp` - Added signal blocking, error checking

Created documentation:
- `SEGFAULT_FIX.md` - Detailed fix instructions
- `MEMORY_OPTIMIZATION.md` - Memory optimization suggestions
- `optimization_example.cpp` - Optimization code examples

## Next Steps Recommendations

Although the program now runs normally, for larger datasets, it's recommended to implement the previously mentioned optimizations:

1. **Add slice caching** - Reduce redundant loading
2. **Reduce image copying** - Utilize QImage implicit sharing
3. **Optimize timers** - Avoid frequent redraws
4. **Automatic downsampling** - Automatically reduce display for very large data

See `MEMORY_OPTIMIZATION.md` and `optimization_example.cpp` for details.
