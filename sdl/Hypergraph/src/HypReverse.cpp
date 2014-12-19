#define TRANSFORM HgReverse
//TODO: name FsX for Fs only xforms? but we can reverse input/output labels on general Hg too.

#define VERSION "v1"

#define HG_TRANSFORM_MAIN




namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION) {}
  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc> *o) {
    reverse(i, o);
    return true;
  }
};




