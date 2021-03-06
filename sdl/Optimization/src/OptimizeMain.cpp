// Copyright 2014-2015 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/**
   \file

   Implements an "Optimize" standalone executable, which calls
   OptimizationProcedure to optimize (i.e., train) the feature weights
   of an SDL module, e.g., the weights of TrainableCapitalizer.

   TODO: test coverage.
 */
#include <sdl/Optimization/MockResourceManager.hpp>
#include <sdl/Config/Config.hpp>
#include <sdl/Config/ConfigureYaml.hpp>
#include <sdl/Optimization/Exception.hpp>
#include <sdl/Optimization/LoadCss.hpp>
#include <sdl/Optimization/OptimizationProcedure.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/Util/Enum.hpp>
#include <sdl/Util/FindFile.hpp>
#include <sdl/Util/InitLogger.hpp>
#include <sdl/Util/InitLoggerFromConfig.hpp>
#include <sdl/Util/IsDebugBuild.hpp>
#include <sdl/Util/LoadLibrary.hpp>
#include <sdl/Util/Locale.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Performance.hpp>
#include <sdl/Util/ProgramOptions.hpp>
#include <sdl/Util/QuickExit.hpp>
#include <sdl/LexicalCast.hpp>
#include <sdl/SharedPtr.hpp>
#include <boost/optional.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace po = boost::program_options;

sdl::Util::DeclareSingleThreadProgram single(true);

sdl::Util::DefaultLocaleFastCout initCout;

std::string getLibraryName(std::string const& name) {
  std::string stem(name);
  stem += "-shared";
#if SDL_IS_DEBUG_BUILD
  stem += "_debug";
#endif
#if defined _WIN64 || defined _WIN32
  stem += ".dll";
  return stem;
#elif defined(__APPLE__)
  stem += ".dylib";
#else
  stem += ".so";
#endif
  return "libsdl-" + stem;
}

int main(int ac, char* av[]) {

  using namespace sdl;
  using namespace sdl::Util;

  try {
    sdl::Util::FinishLoggingAfterScope endLogging;
    po::options_description generic("Command line options");

    std::string configFile;
    std::string modelName, libModel, searchSpaceSectionName, optimizeSectionName;
    bool help;
    bool checkIntersection;
    bool checkGradients;
    std::string logConfigFile;
    bool testMode;
    sdl::AddOption opt(generic);

    opt("config,c", po::value(&configFile)->default_value("./XMTConfig.yml"),
        "SDL config file that describes the DB");
    opt("search-dir,I", po::value<std::vector<std::string>>(), "Directories where config files are searched");
    opt("log-config", po::value(&logConfigFile), "optional log4cxx xml config file");
    opt("model", po::value(&modelName)->default_value("CrfDemo"),
        "Name of the model (specifies shared library to load)");
    opt("libmodel", po::value(&libModel)->default_value(""),
        "Optional exact path of model shared library e.g. /lib/libNE.so (instead of 'model' option)");
    opt("search-space", po::value(&searchSpaceSectionName)->default_value("search-space"),
        "Name of the YAML section that configures the search space");
    opt("optimize", po::value(&optimizeSectionName)->default_value("optimize"),
        "Name of the YAML section that configures the optimizer");
    opt("check-intersection", po::bool_switch(&checkIntersection)->default_value(false),
        "Checks clamped/unclamped intersection (for debugging -- expensive)");
    opt("check-gradients", po::bool_switch(&checkGradients)->default_value(false),
        "Checks gradients computation (for debugging -- expensive)");
    opt("test-mode", po::bool_switch(&testMode)->default_value(false),
        "Test mode (will just decode the data given the weights specified in yaml config file)");
    opt("help,h", po::bool_switch(&help)->default_value(false), "Display help information");

    po::options_description options;
    options.add(generic);

    po::variables_map varMap;
    po::parsed_options parsed = po::command_line_parser(ac, av).options(options).allow_unregistered().run();
    po::store(parsed, varMap);
    std::vector<std::string> unrecognizedArgs((po::collect_unrecognized(parsed.options, po::include_positional)));
    po::notify(varMap);

    if (varMap.count("search-dir")) {
      findFile().setDirs(varMap["search-dir"].as<std::vector<std::string>>());
    }

    initLoggerFromConfig(logConfigFile, "OptimizeMain", Util::kLogInfo);

    if (help) {
      std::cout << generic << '\n';
      Optimization::OptimizationProcedureOptions config;
      std::cout << '\n'
                << "The YAML config file must contain a module of type 'OptimizationProcedure'\n"
                << "with the following options:\n\n";
      Config::showHelp(std::cout, &config);
      return Util::normalExit(0);
    }

    Resources::ResourceManager& resourceManager = Resources::mockResourceManager;

    // Load YAML config file and construct SDL instance (in order to
    // get a ResourceManager)
    ConfigNode configNode = Config::loadConfig(configFile);

    std::string weightsPath;
    for (unsigned i = 0, N = (unsigned)unrecognizedArgs.size(); i < N; ++i) {
      std::string const& arg = unrecognizedArgs[i];
      if (arg.size() > 3 && arg[0] == '-' && arg[1] == '-') {
        std::string::size_type sep = arg.find('=', 2);
        std::string::const_iterator a0 = arg.begin();
        std::string yamlPath, yamlVal;
        if (sep != std::string::npos) {
          yamlPath = std::string(a0 + 2, a0 + sep);
          yamlVal = std::string(a0 + sep + 1, arg.end());
        } else if (i + 1 < N) {
          ++i;
          yamlPath = std::string(a0 + 2, arg.end());
          yamlVal = unrecognizedArgs[i];
        } else
          SDL_THROW_LOG(OptimizeMain, ConfigException, "unrecognized command-line option: " << arg);
        if (!yamlVal.empty()) {
          SDL_INFO(Shell, "config command line override: " << yamlPath << " => " << yamlVal);
          configNode = Config::overrideConfig(configNode, yamlPath, yamlVal);
        }
      } else if (testMode) {
        weightsPath = arg;
      } else {
        SDL_THROW_LOG(OptimizeMain, ConfigException, "unrecognized command-line option: " << arg);
      }
    }

    SDL_DEBUG(Optimization.OptimizeMain, "Config file: " << configFile);
    bool showEffective = true;
    int verbosity = 1;

    Optimization::OptimizationProcedureOptions optProcConfig;
    ConfigNode optimizeNode = configNode[optimizeSectionName];
    if (optimizeNode)
      Config::applyYaml(optimizeNode, &optProcConfig);
    else if (!testMode)
      SDL_THROW_LOG(Optimize, ConfigException, "Could not find config section called '"
                                                   << optimizeSectionName << "' in " << configFile);


    optProcConfig.testMode = testMode;
    if (!weightsPath.empty()) {
      optProcConfig.weightsPath = weightsPath;
    }

    std::string libName = libModel.empty() ? getLibraryName(modelName) : libModel;
    Util::LoadedLib myLib = NULL;
    if (!(myLib = Util::loadLib(libName.c_str())))
      SDL_THROW_LOG(Optimization, IOException, "Could not load shared lib " << libName);

    void* loadFct = NULL;
    if (!(loadFct = Util::loadProc(myLib, "loadCss")))
      SDL_THROW_LOG(Optimization, IOException, "Could not use 'load' function from " << libName);

    ConfigNode node = configNode[searchSpaceSectionName];
    if (!node)
      SDL_THROW_LOG(Optimize, ConfigException, "Could not find config section called '"
                                                   << searchSpaceSectionName << "' in " << configFile);

    shared_ptr<Optimization::CreateSearchSpace> searchSpace{
        ((Optimization::LoadCssFn)loadFct)(resourceManager, node, Optimization::kIsOptimize)};
    SDL_INFO(Optimization, "Loaded search space '" << searchSpace->getName() << "'");

    Optimization::OptimizationProcedure optimizer(searchSpace, optProcConfig);
    if (testMode) {
      optimizer.test();
    } else {
      Util::Performance performance("Optimize.optimize");
      optimizer.setCheckIntersectionNonEmpty(checkIntersection);
      optimizer.setCheckGradients(checkGradients);
      optimizer.optimize();
    }
  } catch (std::exception const& e) {
    std::cerr << e.what() << '\n';
    Util::quickExit(1);
  }
  Util::quickExit(0);
}
