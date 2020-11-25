#pragma once
#include "src/WebServer.h"
#include "src/EventLoop.h"
#include "src/Logging/Logging.h"
#include <memory>


int main()
{
    Logging LOG;
    std::shared_ptr<EventLoop> mainloop = std::make_shared<EventLoop>();
    WebServer server(mainloop);
    server.Start();
    return 0;
}