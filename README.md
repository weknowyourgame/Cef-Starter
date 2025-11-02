# CEF Starter

A minimal, configurable Chromium Embedded Framework (CEF) starter template for building embedded browser applications. This project provides a clean foundation for integrating CEF into your C++ applications, with support for Qt and other GUI frameworks.

## Quick Start

### Prerequisites

- CEF binary distribution (download from [CEF Releases](https://cef-builds.spotifycdn.com/index.html))
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.18 or higher
- X11 development libraries (Linux): `sudo apt-get install libx11-dev` or `sudo yum install libX11-devel`

### Building

1. **Clone or copy this starter template:**
   ```bash
   git clone https://github.com/weknowyourgame/Cef-Starter
   cd Cef-Starter
   ```

2. **Place CEF binary in a standard location:**
   ```bash
   # Option 1: Extract CEF to third_party/cef/
   mkdir -p third_party/cef
   cd third_party/cef
   # Extract cef_binary_XXX_linux64.tar.bz2 here
   
   # Option 2: Set CEF_ROOT environment variable
   export CEF_ROOT=/path/to/cef_binary_XXX_linux64
   ```

3. **Build the project:**
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

4. **Run the application:**
   ```bash
   ./cef_starter
   # Or with a custom URL:
   ./cef_starter --url=https://www.example.com
   ```

## Configuration

The CMake build system supports extensive configuration via command-line variables:

### Project Configuration

- `PROJECT_NAME` - Project name (default: `cef_starter`)
- `EXECUTABLE_NAME` - Output executable name (default: same as `PROJECT_NAME`)

### Application Configuration

- `DEFAULT_URL` - Default URL to load when no `--url` argument is provided (default: `https://www.chromium.org`)
- `WINDOW_TITLE` - Window title bar text (default: same as `PROJECT_NAME`)
- `USE_SANDBOX` - Enable CEF sandbox for enhanced security (default: `OFF`)
- `CEF_ENABLE_OFFSCREEN` - Enable off-screen rendering support (default: `OFF`)

### CEF Path Configuration

- `CEF_ROOT` - Path to CEF binary directory. Auto-detected from:
  - `third_party/cef/cef_binary_*`
  - `../third_party/cef/cef_binary_*`
  - `../../third_party/cef/cef_binary_*`
  - `$ENV{CEF_ROOT}`

### Example: Custom Configuration

```bash
cmake -DPROJECT_NAME=my_browser \
      -DEXECUTABLE_NAME=my_app \
      -DDEFAULT_URL=https://myapp.local \
      -DWINDOW_TITLE="My Application" \
      -DUSE_SANDBOX=ON \
      ..
make
```

## Project Structure

```
cef_starter/
├── main.cpp          # Entry point - CEF initialization and message loop
├── app.h/cpp         # CefApp implementation - creates browser window
├── client.h/cpp      # CefClient implementation - handles browser lifecycle
├── CMakeLists.txt    # CMake build configuration
├── BUILD.bazel       # Bazel build configuration (optional)
└── build/            # Build output directory (gitignored)
```

## Architecture

### Entry Point (`main.cpp`)

Handles the complete CEF lifecycle:
- Sub-process detection and execution
- CEF initialization with configurable settings
- Message loop management
- Clean shutdown

Key responsibilities:
- Sets up resource paths for CEF
- Configures sandbox settings
- Initializes the App instance
- Runs the blocking message loop until shutdown

### Application Handler (`app.h/cpp`)

Implements `CefApp` and `CefBrowserProcessHandler`:
- Creates the browser window when CEF context is ready
- Configures window properties (title, runtime style)
- Handles URL loading (command-line override or default)

The `OnContextInitialized()` callback is where the first browser window is created.

### Client Handler (`client.h/cpp`)

Implements `CefClient` and `CefLifeSpanHandler`:
- Tracks all browser instances
- Handles window close events properly
- Quits the message loop when the last window closes

This ensures clean shutdown when users close all browser windows.

## Runtime Style

The starter uses `CEF_RUNTIME_STYLE_ALLOY` which provides:
- Minimal embedded browser UI
- No Chrome tabs, profiles, or browser chrome
- Just the web content in a native window
- Perfect for embedded applications

To change this, modify `app.cpp`:
```cpp
window_info.runtime_style = CEF_RUNTIME_STYLE_CHROME;  // Full Chrome UI
// or
window_info.runtime_style = CEF_RUNTIME_STYLE_DEFAULT; // Default style
```

## Extending the Starter

### Adding More CEF Handlers

To add functionality like navigation events, loading callbacks, or display updates:

1. **Extend the Client class** (`client.h`):
   ```cpp
   class Client : public CefClient,
                  public CefLifeSpanHandler,
                  public CefDisplayHandler,  // Add navigation/title updates
                  public CefLoadHandler {     // Add loading state callbacks
   public:
     // CefClient methods
     CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
     CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
     
     // CefDisplayHandler methods
     void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
     
     // CefLoadHandler methods
     void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, 
                      TransitionType transition_type) override;
   };
   ```

2. **Implement the handlers** in `client.cpp`

### Adding Command-Line Options

Modify `main.cpp` to parse additional arguments:

```cpp
CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();

if (command_line->HasSwitch("debug")) {
  // Enable debug mode
}

std::string custom_arg = command_line->GetSwitchValue("custom");
if (!custom_arg.empty()) {
  // Use custom argument
}
```

### Customizing Browser Settings

Modify `app.cpp` in `OnContextInitialized()`:

```cpp
CefBrowserSettings browser_settings;
browser_settings.plugins = STATE_DISABLED;  // Disable plugins
browser_settings.javascript = STATE_ENABLED;  // Enable JavaScript
browser_settings.web_security = STATE_ENABLED;  // Enable web security
```

## Integration with GUI Frameworks

### Qt Integration

This starter is designed to work with Qt. Here are integration approaches:

#### Option 1: Separate Thread (Recommended)

Run CEF in a separate thread for best isolation:

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
  void initializeCEF();
  void messageLoopThread();
  
  CefRefPtr<Client> client_;
  CefRefPtr<CefBrowser> browser_;
  QThread* cef_thread_;
};
```

#### Option 2: Timer-Based Integration

Use a QTimer to periodically process CEF messages:

```cpp
class QCefWidget : public QWidget {
  Q_OBJECT
private slots:
  void processCEFMessages() {
    CefDoMessageLoopWork();
  }
  
private:
  QTimer* message_timer_;
};

// In constructor:
message_timer_ = new QTimer(this);
connect(message_timer_, &QTimer::timeout, this, &QCefWidget::processCEFMessages);
message_timer_->start(10);  // Every 10ms
```

#### Option 3: Off-Screen Rendering (OSR)

For more control, use off-screen rendering:

1. Enable OSR in CMake: `-DCEF_ENABLE_OFFSCREEN=ON`
2. Implement `CefRenderHandler` in your Client
3. Render to QPixmap/QImage and display in a QLabel

### Embedding in Native Windows

To embed the CEF window in an existing native window:

1. Get the window handle from CEF after browser creation
2. Reparent the window using platform-specific APIs
3. Handle resize events to update browser bounds

## Troubleshooting

### CEF Not Found

If CMake can't find CEF:
```bash
cmake -DCEF_ROOT=/absolute/path/to/cef_binary_XXX_linux64 ..
```

### Build Errors

- **"CEF_INCLUDE_DIR not found"**: Ensure CEF_ROOT points to the correct directory containing `include/` subdirectory
- **"Unknown CMake command SET_LIBRARY_TARGET_PROPERTIES"**: This is a CEF CMake issue - ensure CMAKE_BUILD_TYPE is set before project()
- **Missing X11 libraries**: Install `libx11-dev` (Debian/Ubuntu) or `libX11-devel` (RedHat/CentOS)

### Runtime Issues

- **"Error loading V8 startup snapshot file"**: Ensure `v8_context_snapshot.bin` is copied to the build directory (handled automatically by CMake)
- **"GPU process launch failed"**: This is often harmless for basic browsing. To disable GPU acceleration, add to browser settings:
  ```cpp
  browser_settings.chrome_status = STATE_DISABLED;
  ```

### Window Not Appearing

- Check that X11 is available: `echo $DISPLAY`
- Ensure window manager is running
- Check console output for error messages

## Building for Other Platforms

### Windows

The CMakeLists.txt includes Windows-specific configuration. You'll need:
- Visual Studio 2017 or later
- CEF Windows binary distribution
- Set `CEF_ROOT` to the Windows CEF directory

### macOS

macOS support is configured in CMakeLists.txt. Requirements:
- Xcode with command-line tools
- CEF macOS binary distribution
- Set `CEF_ROOT` appropriately

## Requirements

- CEF binary distribution matching your platform
- C++17 compatible compiler
- CMake 3.18+
- Platform-specific dependencies:
  - Linux: X11 development libraries
  - Windows: Visual Studio runtime
  - macOS: Xcode command-line tools

## License

This starter template follows the same license as CEF (BSD-style). See your CEF distribution's LICENSE file for details.

## Resources

- [CEF Documentation](https://bitbucket.org/chromiumembedded/cef/wiki/Home.md)
- [CEF General Usage Guide](https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage.md)
- [CEF API Documentation](https://cef-builds.spotifycdn.com/docs/index.html)
- [CEF Binary Downloads](https://cef-builds.spotifycdn.com/index.html)

## Contributing

This is a starter template - feel free to fork and customize for your needs. If you have improvements that would benefit others, contributions are welcome.

## Version

This starter is compatible with CEF 139+ (Chromium 139+). For older CEF versions, some API calls may need adjustment.
