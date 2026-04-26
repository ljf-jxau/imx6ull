#include <iostream>
#include "server.h"

int main()
{
    Server s;
    s.listen(IP,PORT);

    return 0;

}