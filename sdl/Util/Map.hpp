















#include <utility>
#include <stdexcept>
















































































}







template <class Map>

























































































































































  return i.first->second;
}

template <class Set, class K>
inline typename Set::value_type const& value(Set& s, K const& k) {
  return *s.find(k);
}

template <class Set, class K, class V>

  return s[k] = v;  // instead of insert, because this is faster than value_type(k, v) for expensive k
}




































  out.clear();
  if (in2.size() < in1.size()) {
    intersectInto(out, in2, in1);
    return;
  }
  for (typename US::const_iterator i = in1.begin(); i != in1.end(); i++) {











  out.clear();
  out.insert(in1.begin(), in1.end());
  out.insert(in2.begin(), in2.end());
}



































// adds default val to table if key wasn't found, returns ref to val
template <class H, class K>
typename H::mapped_type& get_default(H& ht, K const& k, typename H::mapped_type const& v) {
  return const_cast<typename H::mapped_type&>(ht.insert(typename H::value_type(k, v)).first->second);
}

// like ht[k]=v, but you want to check your belief that ht[k] didn't exist before.  also may be faster
template <class HT, class K, class V>



}

// NB: value is in/out param
template <class HT, class K, class V>

  std::pair<typename HT::iterator, bool> r = ht.insert(typename HT::value_type(key, value));

  return r.second;
}




































#endif
