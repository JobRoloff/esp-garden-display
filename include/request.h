// request.h
#pragma once
#include <Arduino.h>

// Returns true on success, fills outBody and outStatus
bool http_get(const char* url, String& outBody, int& outStatus);
