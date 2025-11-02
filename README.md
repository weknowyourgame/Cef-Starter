# CEF Starter

A Chromium Embedded Framework (CEF) starter application designed as a foundation for integrating CEF with Qt and other GUI frameworks.

## Overview

This is the simplest possible CEF application that:
- Initializes CEF
- Creates a single browser window
- Handles window lifecycle (open/close)
- Uses native windows (no Views framework)
- Provides a clean foundation for Qt integration

## Features

- **Minimal Code**: Less than 300 lines total
- **Native Windows**: Uses CEF's native window mode (easier to integrate with Qt)
- **Cross-Platform**: Linux support (Windows/macOS structure ready)
- **Simple API**: Only essential CEF components
- **Command-Line URL**: Override default URL with `--url=<url>`

## File Structure

```
cef_starter/
├── main.cpp          # Entry point - CEF initialization and message loop
├── app.h/cpp         # CefApp implementation - creates browser window
├── client.h/cpp      # CefClient implementation - handles browser lifecycle
├── CMakeLists.txt    # CMake build configuration
├── BUILD.bazel       # Bazel build configuration
└── linux/            # Platform-specific files
    └── BUILD.bazel
```

## Building

### Using CMake

Basic build:
```bash
cd cef_starter
mkdir build && cd build
cmake ..
make
```

The executable will be in `build/` directory.

#### Configuration Options

The CMake build system supports several configuration options:

**Project Configuration:**
- `PROJECT_NAME` - Project name (default: `cef_starter`)
- `EXECUTABLE_NAME` - Executable name (default: same as `PROJECT_NAME`)

**Application Configuration:**
- `DEFAULT_URL` - Default URL to load (default: `https://www.chromium.org`)
- `WINDOW_TITLE` - Window title (default: same as `PROJECT_NAME`)
- `USE_SANDBOX` - Enable CEF sandbox (default: `OFF`)
- `CEF_ENABLE_OFFSCREEN` - Enable off-screen rendering support (default: `OFF`)

**CEF Configuration:**
- `CEF_ROOT` - Path to CEF binary directory (auto-detected from common locations)

**Example with custom configuration:**
```bash
cmake -DPROJECT_NAME=my_app \
      -DEXECUTABLE_NAME=my_browser \
      -DDEFAULT_URL=https://example.com \
      -DWINDOW_TITLE="My Browser" \
      -DUSE_SANDBOX=ON \
      ..
make
```

Or set CEF_ROOT explicitly:
```bash
cmake -DCEF_ROOT=/path/to/cef_binary_XXX ..
make
```

### Using Bazel

```bash
bazel build //cef_starter:cef_starter
```

## Usage

Run the application:

```bash
./cef_starter
```

Or specify a custom URL:

```bash
./cef_starter --url=https://www.example.com
```

## Qt Integration Notes

This starter is designed to be integrated with Qt. Here are the key considerations:

### Event Loop Integration

CEF requires its message loop to run. You have several options:

1. **Separate Thread** (Recommended):
   - Run CEF in a separate thread
   - Use Qt's event system for communication
   - Best isolation and performance

2. **Timer-Based Integration**:
   - Use `QTimer` to periodically call `CefDoMessageLoopWork()`
   - Call it every 10-20ms
   - Simpler but less efficient

3. **Custom Message Loop**:
   - Replace Qt's event loop with CEF's (complex, not recommended)

### Rendering Modes

1. **Native Child Window (NCW)** - Current Mode:
   - CEF creates a native window
   - Embed the window handle in a Qt widget
   - Platform-specific window handle embedding required
   - Better performance

2. **Off-Screen Rendering (OSR)** - Future Enhancement:
   - CEF renders to bitmap
   - Display bitmap in Qt widget
   - Easier integration
   - Lower performance

### Integration Steps

1. **Extract CEF Initialization**:
   - Move CEF initialization to a separate class
   - Allow initialization without creating a window immediately

2. **Create Qt Widget Wrapper**:
   - Create a QWidget that embeds the CEF window handle
   - Handle window resizing
   - Integrate CEF message loop with Qt event loop

3. **Add OSR Mode** (Optional):
   - Implement off-screen rendering
   - Render to QPixmap/QImage
   - Display in QLabel or custom widget

4. **Handle CEF Callbacks**:
   - Connect CEF callbacks to Qt signals/slots
   - Handle navigation, loading, etc. in Qt code

### Example Integration Pattern

```cpp
class QCefWidget : public QWidget {
  Q_OBJECT
public:
  QCefWidget(QWidget* parent = nullptr);
  ~QCefWidget();
  
  void loadUrl(const QString& url);
  
signals:
  void titleChanged(const QString& title);
  void urlChanged(const QString& url);
  
private:
  CefRefPtr<Client> client_;
  CefRefPtr<CefBrowser> browser_;
  
  // Timer to pump CEF message loop
  QTimer* message_timer_;
  
private slots:
  void onMessageTimer();
};
```

## Code Organization

### `main.cpp`
- Entry point
- Handles sub-process execution
- Initializes CEF
- Runs message loop
- Shuts down CEF

### `app.h/cpp`
- Implements `CefApp` and `CefBrowserProcessHandler`
- Creates the first browser window in `OnContextInitialized()`
- Uses native window mode (no Views framework)

### `client.h/cpp`
- Implements `CefClient` and `CefLifeSpanHandler`
- Tracks browser instances
- Handles window close events
- Quits message loop when last window closes

## Extending

### Adding More Handlers

To add more functionality, extend the `Client` class:

```cpp
class Client : public CefClient,
               public CefLifeSpanHandler,
               public CefDisplayHandler,  // Add this
               public CefLoadHandler {     // Add this
  // Implement handler methods
};
```

### Adding Command-Line Options

Modify `main.cpp` to parse additional command-line arguments:

```cpp
if (command_line->HasSwitch("my-option")) {
  // Handle option
}
```

### Changing Default URL

Modify `app.cpp`:

```cpp
if (url.empty()) {
  url = "https://your-default-url.com";
}
```

## Requirements

- CEF libraries (libcef.so/libcef.dll/libcef.dylib)
- CEF headers
- C++17 compatible compiler
- CMake 3.16+ or Bazel

## Platform Support

- ✅ **Linux**: Fully supported
- 🔄 **Windows**: Structure ready, needs implementation
- 🔄 **macOS**: Structure ready, needs implementation

## Next Steps

1. Test the basic functionality
2. Review Qt integration approaches
3. Consider adding OSR mode support
4. Create Qt widget wrapper (separate project)

## License

Same as CEF - BSD-style license. See LICENSE file in parent directory.

## References

- [CEF Documentation](https://bitbucket.org/chromiumembedded/cef/wiki/Home.md)
- [CEF General Usage](https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage.md)
- [QCefView](https://github.com/CefView/QCefView) - Qt widget wrapper for CEF

