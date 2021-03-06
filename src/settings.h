#ifndef OPTIONS_H_
#define OPTIONS_H_

#include "./colors.h"
#include "./helper.h"
#include "./worldloader.h"
#include <cstdint>
#include <filesystem>
#include <string>
#define UNDEFINED 0x7FFFFFFF

namespace Settings {

struct Dimension {
  string ns, id;

  Dimension(string ns, string id) : ns(ns), id(id){};
  Dimension(string _id) : ns("minecraft") {
    size_t sep = _id.find_first_of(':');

    if (sep == std::string::npos) {
      // If there is no ':', this is just an id
      id = _id;
    } else {
      // If not, add each part separately
      ns = _id.substr(0, sep);
      id = _id.substr(sep + 1);
    }
  };

  std::filesystem::path regionDir(std::filesystem::path savePath) {
    if (id == "overworld")
      return savePath /= std::filesystem::path("region");
    else if (id == "the_nether")
      return savePath /= std::filesystem::path("DIM-1/region");
    else if (id == "the_end")
      return savePath /= std::filesystem::path("DIM1/region");
    else
      return savePath /= std::filesystem::path(
                 fmt::format("dimensions/{}/{}/region", ns, id));
  };

  string to_string() { return fmt::format("{}:{}", ns, id); };
};

struct WorldOptions {
  // Files to use
  std::filesystem::path saveName, outFile, colorFile;

  // Map boundaries
  Dimension dim;
  std::string customDim;
  Coordinates boundaries;
  uint16_t splits;

  // Image settings
  uint16_t padding; // Should be enough
  bool hideWater, hideBeacons, shading;

  // Marker storage
  uint8_t totalMarkers;
  Colors::Marker markers[256];

  // Memory limits, legacy code for image splitting
  int offsetY;
  uint64_t memlimit;
  bool memlimitSet, wholeworld;

  WorldOptions() : dim("overworld") {
    saveName = "";
    outFile = "output.png";
    colorFile = "colors.json";

    splits = 1;
    boundaries.setUndefined();
    boundaries.minY = 0;
    boundaries.maxY = 255;

    offsetY = 3;
    hideWater = hideBeacons = shading = false;
    padding = 5;

    totalMarkers = 0;

    wholeworld = false;
    memlimit = 2000 * uint64_t(1024 * 1024);
    memlimitSet = false;
  }

  std::filesystem::path regionDir() { return dim.regionDir(saveName); }
};

bool parseArgs(int argc, char **argv, Settings::WorldOptions *opts);

} // namespace Settings

#endif // OPTIONS_H_
