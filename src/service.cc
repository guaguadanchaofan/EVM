#include "service.h"


void Service::start()
{
    network.start();
}

void Service::stop()
{
    network.stop();
}
