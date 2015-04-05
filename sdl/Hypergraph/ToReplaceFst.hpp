// Copyright 2014 SDL plc
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/** \file

    an openfst replacefst is like a RTN or CFG

    warning: drawing a replaceFst will infinite-loop unless the Hg is finite (no
    cycles) - an infinite cfg whose language isn't finite can't be completely expanded as a replacefst - it can reach an infinite number of states (stacks)

    e.g. try HgFsmDraw --replaceFst 1 RegressionTests/Hypergraph3/lm1-fst.hgtxt

    this hangs

    but HgFsmDraw --replaceFst 0 RegressionTests/Hypergraph3/lm1-fst.hgtxt doesn't
*/

#ifndef HYP__HYPERGRAPH_TO_REPLACE_FST_HPP
#define HYP__HYPERGRAPH_TO_REPLACE_FST_HPP
#pragma once

#if HAVE_OPENFST
#include <sdl/Hypergraph/UseOpenFst.hpp>
#include <sdl/LexicalCast.hpp>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>

#include <sdl/Util/Forall.hpp>
#include <sdl/Util/LogHelper.hpp>

#include <utility>
#include <queue>
#include <set>

namespace sdl {
namespace Hypergraph {

/**
   Converts hypergraph to OpenFst ReplaceFst (which is an
   RTN). Hypergraph must store incoming arcs.
*/
template<class FstArc, class HgArc>
fst::ReplaceFst<FstArc>*
toReplaceFst(IHypergraph<HgArc> const& hg, fst::SymbolTable* fsyms) {
  if (!hg.storesInArcs()) {
    SDL_THROW_LOG(Hypergraph.ToReplaceFst, std::runtime_error,
                  "Needs incoming arcs");
  }
  std::queue<StateId> queue;
  std::set<StateId> onQueue;

  StateId finalState = hg.final();
  queue.push(finalState);
  onQueue.insert(finalState);

  typedef typename FstArc::Label FLabel;
  typedef typename FstArc::StateId FStateId;
  typedef typename FstArc::Weight FWeight;
  typedef std::pair<FLabel, const fst::Fst<FstArc>*> LabelFstPair;
  std::vector<LabelFstPair> fsts;

  fsyms->AddSymbol(EPSILON::TOKEN); // in OpenFst, eps must be first (=0)
  std::string root =
      "nonterminal_#" + sdl::lexical_cast<std::string>(finalState);
  FLabel fRootLabel = fsyms->AddSymbol(root);

  while (!queue.empty()) {
    StateId sid = queue.front();
    SDL_DEBUG(Hypergraph.ToReplaceFst,
              "Converting HG state " << sid << " to FST");
    queue.pop();

    // Each HG state gets an FST "nonterminal"
    std::string token = "nonterminal_#" + sdl::lexical_cast<std::string>(sid);
    FLabel nonterm = fsyms->AddSymbol(token);
    fst::MutableFst<FstArc>* fst = new fst::VectorFst<FstArc>();
    FStateId startState = fst->AddState();
    fst->SetStart(startState);

    // Each HG arc has tails, which are converted into a sequence in an FST
    forall (ArcId aid, hg.inArcIds(sid)) {
      HgArc* arc = hg.inArc(sid, aid);
      FStateId prevState = startState;
      for (std::size_t t = 0, end = arc->getNumTails(); t < end; ++t) {
        StateId tailId = arc->getTail(t);
        FLabel flabel;
        std::string token;
        if (hg.hasLexicalLabel(tailId)) {
          token = hg.getVocabulary()->str(hg.inputLabel(tailId));
        }
        else {
          // Nonterminal: Gets its own Fst
          token = "nonterminal_#" + sdl::lexical_cast<std::string>(tailId);
          if (onQueue.find(tailId) == onQueue.end()) {
            queue.push(tailId);
            onQueue.insert(tailId);
          }
        }
        flabel = fsyms->AddSymbol(token);
        FStateId nextState = fst->AddState();
        FWeight fweight =
            (t == 0) ? FWeight(arc->weight().getValue()) : FWeight::One();
        fst->AddArc(prevState, FstArc(flabel, flabel, fweight, nextState));
        prevState = nextState;
      }
      fst->SetFinal(prevState, FWeight::One());
    }

    fsts.push_back(LabelFstPair(nonterm, fst));
  }
  SDL_DEBUG(Hypergraph.ToReplaceFst, "Done.");

  fst::ReplaceFst<FstArc>* result =
      new fst::ReplaceFst<FstArc>(fsts, fRootLabel);
  // return std::make_pair(result, fsyms);
  return result;
}

template<class FstArc, class HgArc>
std::pair<fst::ReplaceFst<FstArc>*, fst::SymbolTable*>
toReplaceFst(IHypergraph<HgArc> const& hg) {
  fst::SymbolTable* syms = new fst::SymbolTable("");
  fst::ReplaceFst<FstArc>* result = toReplaceFst<FstArc>(hg, syms);
  return std::make_pair(result, syms);
}


}}

#endif // if HAVE_OPENFST

#endif
