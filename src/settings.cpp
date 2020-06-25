#include "./settings.h"

#define ISPATH(p) (!(p).empty() && std::filesystem::exists((p)))

bool Settings::parseArgs(int argc, char **argv, Settings::WorldOptions *opts) {
#define MOREARGS(x) (argpos + (x) < argc)
#define NEXTARG argv[++argpos]
#define POLLARG(x) argv[argpos + (x)]
  int argpos = 0;
  while (MOREARGS(1)) {
    const char *option = NEXTARG;
    if (strcmp(option, "-from") == 0) {
      if (!MOREARGS(2) || !isNumeric(POLLARG(1)) || !isNumeric(POLLARG(2))) {
        fprintf(stderr, "Error: %s needs two integer arguments\n", option);
        return false;
      }
      opts->boundaries.minX = atoi(NEXTARG);
      opts->boundaries.minZ = atoi(NEXTARG);
    } else if (strcmp(option, "-to") == 0) {
      if (!MOREARGS(2) || !isNumeric(POLLARG(1)) || !isNumeric(POLLARG(2))) {
        fprintf(stderr, "Error: %s needs two integer arguments\n", option);
        return false;
      }
      opts->boundaries.maxX = atoi(NEXTARG);
      opts->boundaries.maxZ = atoi(NEXTARG);
    } else if (strcmp(option, "-max") == 0) {
      if (!MOREARGS(1) || !isNumeric(POLLARG(1))) {
        fprintf(stderr, "Error: %s needs an integer argument\n", option);
        return false;
      }
      const int height = atoi(NEXTARG);
      opts->boundaries.maxY =
          (height > MAX_TERRAIN_HEIGHT ? MAX_TERRAIN_HEIGHT : height);
    } else if (strcmp(option, "-min") == 0) {
      if (!MOREARGS(1) || !isNumeric(POLLARG(1))) {
        fprintf(stderr, "Error: %s needs an integer argument\n", option);
        return false;
      }
      const int height = atoi(NEXTARG);
      opts->boundaries.minY =
          (height < MIN_TERRAIN_HEIGHT ? MIN_TERRAIN_HEIGHT : height);
    } else if (strcmp(option, "-splits") == 0) {
      if (!MOREARGS(1) || !isNumeric(POLLARG(1))) {
        fprintf(stderr, "Error: %s needs an integer argument\n", option);
        return false;
      }
      opts->splits = atoi(NEXTARG);
    } else if (strcmp(option, "-padding") == 0) {
      if (!MOREARGS(1) || !isNumeric(POLLARG(1)) || atoi(POLLARG(1)) < 0) {
        fprintf(stderr, "Error: %s needs an positive integer argument\n",
                option);
        return false;
      }
      opts->padding = atoi(NEXTARG);
    } else if (strcmp(option, "-nowater") == 0) {
      opts->hideWater = true;
    } else if (strcmp(option, "-nobeacons") == 0) {
      opts->hideBeacons = true;
    } else if (strcmp(option, "-shading") == 0) {
      opts->shading = true;
    } else if (strcmp(option, "-nether") == 0) {
      opts->dim = Dimension::NETHER;
    } else if (strcmp(option, "-end") == 0) {
      opts->dim = Dimension::END;
    } else if (strcmp(option, "-file") == 0) {
      if (!MOREARGS(1)) {
        fprintf(stderr, "Error: %s needs one argument\n", option);
        return false;
      }
      opts->outFile = NEXTARG;
    } else if (strcmp(option, "-colors") == 0) {
      if (!MOREARGS(1)) {
        fprintf(stderr, "Error: %s needs one argument\n", option);
        return false;
      }
      opts->colorFile = NEXTARG;
      if (!ISPATH(opts->colorFile)) {
        fprintf(stderr, "Error: File %s does not exist\n",
                opts->colorFile.c_str());
        return false;
      }
    } else if (strcmp(option, "-marker") == 0) {
      if (!MOREARGS(3) || !(isNumeric(POLLARG(1)) && isNumeric(POLLARG(2)))) {
        fprintf(stderr, "Error: %s needs three arguments: x z color\n", option);
        return false;
      }
      int x = atoi(NEXTARG), z = atoi(NEXTARG);
      opts->markers[opts->totalMarkers++] =
          Colors::Marker(x, z, std::string(NEXTARG));
    } else if (strcmp(option, "-nw") == 0) {
      opts->boundaries.orientation = NW;
    } else if (strcmp(option, "-sw") == 0) {
      opts->boundaries.orientation = SW;
    } else if (strcmp(option, "-ne") == 0) {
      opts->boundaries.orientation = NE;
    } else if (strcmp(option, "-se") == 0) {
      opts->boundaries.orientation = SE;
    } else if (strcmp(option, "-3") == 0) {
      opts->offsetY = 3;
    } else if (strcmp(option, "-help") == 0 || strcmp(option, "-h") == 0) {
      return false;
    } else {
      opts->saveName = std::filesystem::path(option);
      if (!ISPATH(opts->saveName)) {
        fprintf(stderr, "Error: File %s does not exist\n",
                opts->saveName.c_str());
        return false;
      }
    }
  }

  // Scan the region directory and map the existing terrain in this set of
  // coordinates
  Coordinates existingWorld;
  scanWorldDirectory(opts->regionDir(), &existingWorld);

  if (opts->boundaries.isUndefined()) {
    // No boundaries were defined, import the whole existing world
    // No overwriting to preserve potential min/max data
    opts->boundaries.minX = existingWorld.minX;
    opts->boundaries.minZ = existingWorld.minZ;
    opts->boundaries.maxX = existingWorld.maxX;
    opts->boundaries.maxZ = existingWorld.maxZ;
  } else {
    // Restrict the map to draw to the existing terrain
    opts->boundaries.crop(existingWorld);
  }

  if (opts->boundaries.maxX < opts->boundaries.minX ||
      opts->boundaries.maxZ < opts->boundaries.minZ) {
    fprintf(stderr, "Nothing to render: -from X Z has to be <= -to X Z\n");
    return false;
  }

  if (opts->boundaries.maxX - opts->boundaries.minX < 0) {
    fprintf(stderr, "Nothing to render: -min Y has to be < -max Y\n");
    return false;
  }

  size_t length = opts->boundaries.maxX - opts->boundaries.minX + 1;
  if (opts->splits > length) {
    fprintf(stderr, "Cannot split terrain in more than %ld units.\n", length);
    return false;
  }

  return true;
}