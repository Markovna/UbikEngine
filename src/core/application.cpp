#include "application.h"

application* g_application;

void set_application(application* app) {
  g_application = app;
}

void remove_application() {
  delete g_application;
}


