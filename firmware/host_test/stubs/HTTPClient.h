#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include "WiFiClientSecure.h"
struct HTTPClient {
  std::istringstream body;
  bool begin(WiFiClientSecure &, const char *url) { printf("GET %s\n", url); return true; }
  void setTimeout(int) {}
  void useHTTP10(bool) {}
  int GET() { std::ifstream f("sample_openmeteo.json"); std::stringstream ss; ss << f.rdbuf(); body.str(ss.str()); return 200; }
  std::istringstream &getStream() { return body; }
  void end() {}
};
