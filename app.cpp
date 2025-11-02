#include "app.h"

#include <string>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"
#include "client.h"

// Default values if not provided via CMake
#ifndef DEFAULT_URL
#define DEFAULT_URL "https://www.chromium.org"
#endif

#ifndef WINDOW_TITLE
#define WINDOW_TITLE "CEF Starter"
#endif

App::App() = default;

void App::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();

  // Client implements browser-level callbacks.
  CefRefPtr<Client> handler(new Client);

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;

  // Determine URL to load.
  std::string url = command_line->GetSwitchValue("url");
  if (url.empty()) {
    // Use default URL from CMake configuration
    // CMake passes this as a string literal, so use it directly
    url = DEFAULT_URL;
  }

  // Information used when creating the native window.
  CefWindowInfo window_info;

#if defined(OS_WIN)
  // On Windows we need to specify certain flags that will be passed to
  // CreateWindowEx().
  window_info.SetAsPopup(nullptr, WINDOW_TITLE);
#elif defined(CEF_X11)
  // On Linux/X11, create a top-level window
  // CEF will automatically create the window - we just need to provide basic info
  // Leave window_info mostly default - CEF handles window creation on Linux
#endif

  // Use Alloy runtime style - minimal embedded browser without Chrome UI
  window_info.runtime_style = CEF_RUNTIME_STYLE_ALLOY;

  // Create the first browser window using native window mode.
  CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings,
                                nullptr, nullptr);
}

