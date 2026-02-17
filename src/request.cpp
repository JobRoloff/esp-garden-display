#include "request.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

static WiFiClientSecure secureClient;

bool http_get(const char* url, String& outBody, int& outStatus) {
  outBody = "";
  outStatus = -1;

  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  HTTPClient http;
  http.setTimeout(8000);

  // HTTPS vs HTTP
  const bool isHttps = (strncmp(url, "https://", 8) == 0);

  if (isHttps) {
    secureClient.setInsecure();              // dev-only; use certs for production
    if (!http.begin(secureClient, url)) {
      return false;
    }
  } else {
    if (!http.begin(url)) {
      return false;
    }
  }

  int code = http.GET();
  outStatus = code;

  if (code > 0) {
    outBody = http.getString();
  }

  http.end();
  return (code >= 200 && code < 300);
}
