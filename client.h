#ifndef CEF_STARTER_CLIENT_H_
#define CEF_STARTER_CLIENT_H_

#include <list>

#include "include/cef_client.h"

// Minimal client implementation - only handles browser lifecycle.
class Client : public CefClient, public CefLifeSpanHandler {
 public:
  Client();
  ~Client() override;

  // CefClient methods:
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

  // CefLifeSpanHandler methods:
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

 private:
  // List of existing browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
  BrowserList browser_list_;

  bool is_closing_ = false;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(Client);
};

#endif  // CEF_STARTER_CLIENT_H_

