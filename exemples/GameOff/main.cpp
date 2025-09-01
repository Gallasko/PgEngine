#include <iostream>

#include "application.h"

int main(int argc, char *argv[])
{
    // Decouple C++ and C stream for faster runtime
    std::ios_base::sync_with_stdio(false);
    
    GameApp app("Forgotten Archivist - Idle Game");

    return app.exec();
}