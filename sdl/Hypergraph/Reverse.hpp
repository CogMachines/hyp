








   e.g. (ab)c(d(ef)) => [ba]c(d[fe]) -> [ba]c[fed] -> fedcba

   //TODO: fsm reverse in place - requires swapping inarcs, outarcs indices?. for now copies.
   */





#include <algorithm>










namespace Hypergraph {

// TODO: This would be a good test case for a lazy (on-demand)
// operation.

namespace fs {

template <class Arc>
struct ArcReverser {






};

template <class Arc>
struct ArcReverserFsInPlace {



    typename Arc::StateIdContainer& tails = arc.tails();

    arc.setHead(tails[0]);
    tails[0] = h;
  }

};

template <class Arc>






  }
  result->setVocabulary(inhg.getVocabulary());



  ArcReverser<Arc> fct(result);

}

template <class Arc>
void reverseFsm(IMutableHypergraph<Arc>& h) {

  h.clear();


}


template <class Arc>
struct ArcReverserCfgInPlace {



    typename Arc::StateIdContainer& tails = arc.tails();
    std::reverse(tails.begin(), tails.end());
  }

};

template <class Arc>
void reverseCfg(IMutableHypergraph<Arc>& h) {
  if (h.prunedEmpty()) return;



  ArcReverserCfgInPlace<Arc> rev(&h);

}

template <class Arc>

  if (inhg.isFsm())
    fs::reverseFsm(inhg, result);
  else {
    copyHypergraph(inhg, result);
    reverseCfg(*result);
  }
}

template <class Arc>
void reverse(IMutableHypergraph<Arc>& h) {
  if (h.isFsm())
    fs::reverseFsm(h);
  else
    reverseCfg(h);
}










  template <class A>
  void inplace(IMutableHypergraph<A>& h) const {
    reverse(h);
  }
};




#endif