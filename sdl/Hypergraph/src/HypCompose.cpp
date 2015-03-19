#define SDL_TRANSFORM_MAIN_LOG_WEIGHT 1
#define SDL_TRANSFORM_MAIN_EXPECTATION_WEIGHT 1
#define HG_MAIN
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Compose.hpp>

namespace sdl {
namespace Hypergraph {

#define USAGE "Compose cfg*fsm*...*fsm"
#define VERSION "v1"

struct HypCompose : TransformMain<HypCompose> {
  static bool nbestHypergraphDefault() { return false; } // for backward compat w/ regtests mostly

  HypCompose()
      : TransformMain<HypCompose>("HypCompose", USAGE, VERSION)
  {
    opt.require_ins();
    composeOpt.addFstOption = false;
    composeOpt.fstCompose = true;
  }

  void declare_configurable() {
    this->configurable(&composeOpt);
  }

  static BestOutput bestOutput() { return kBestOutput; }
  static LineInputs lineInputs() { return kNoLineInputs; }

  ComposeTransformOptions composeOpt;

  Properties properties(int i) const {  //0 is out, 1 is cfg, 2 and on are all fsms
    return kDefaultProperties
        | kStoreFirstTailOutArcs
        ;
  }

  enum {
    has_transform1 = false,
    has_transform2 = true,
  };

  char const* transform2sep() const { return " * "; }

  template <class Arc>
  bool transform2mm(IMutableHypergraph<Arc> & hg1, IMutableHypergraph<Arc> & hg2, IMutableHypergraph<Arc> *result) {
    ComposeTransform<Arc> c(composeOpt);
    c.setFst(hg2);
    c.inout(hg1, result);
    return true;
  }

};


}}

INT_MAIN(sdl::Hypergraph::HypCompose)
