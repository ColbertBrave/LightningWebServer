#pragma once
#include "WebServer.h"
#include "EventLoop.h"
#include "Logging/Logging.h"
#include <memory>

int main()
{
    std::shared_ptr<EventLoop> mainloop = std::make_shared<EventLoop>();
    WebServer server(mainloop);
    server.Start();
    return 0;
}