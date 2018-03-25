#include "application/application.hpp"

#ifdef main
# undef main
#endif

int main(int argc, const char* argv[]) {
    Application app;
    app.start(argc, argv);
}