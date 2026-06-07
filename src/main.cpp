#include <raylib.h> // necessary to render

#include "core/dimensions.h" // necessary for the dimensions (width, height, title)
#include "core/game.h" // necessary for the actual Application, utils (via `utils.h`) and logic (via `logic.h`)
#include "utils.h"

// TODO: in the near future, add a argc, argv for
//      gui (normal), or cli
int main(int argc, char **argv) {
  if (argc > 1 && (Vendor::ypkg::findOption(argv, argc, "--help") == 0 ||
                   Vendor::ypkg::findOption(argv, argc, "-h") == 0 ||
                   Vendor::ypkg::findOption(argv, argc, "help") == 0)) {
    std::cout << "Usage:\n"
              << "  `chessy [OPTIONS] [COMMANDS]`\n"
              << "  or \n"
              << "  `chessy` # as a standalone gui\n"
              << "\n";
    std::cout << "Options: \n"
              << "  -h --help        Displays this help page\n"
              << "  -v --version     Displays the version of `chessy`\n\n"
              << "Commands: \n"
              << "  help             Displays the help page (same as `chessy "
                 "--help`)\n";

    std::cout << "Returns: \n"
              << "  SUCCESS or [code 0] if there are no errors.\n";
    return 0;
  } else if (Vendor::ypkg::findOption(argv, argc, "version") == 0 ||
             Vendor::ypkg::findOption(argv, argc, "-v") == 0 ||
             Vendor::ypkg::findOption(argv, argc, "--version") == 0) {
    std::cout << "chessy version: v1.0.1\n";
    return 0;
  }

  ChessApplication app(width, height, title);

  app.run(); // run the application

  return 0;
}
