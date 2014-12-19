




























































// Boost for Windows has a TSS bug. Probably fixed in 1.52
// https://svn.boost.org/trac/boost/ticket/5696
#if BOOST_VERSION < 105200 && defined(_WIN32)

shared_ptr<impl::tss_cleanup_function> makeDummyTssCleanupFunction();


#endif




































































# else
    // Boost (before 1.52) bug on Windows: The cleanup function must be provided.
    impl::set_tss_data(this, makeDummyTssCleanupFunction(), (void*)static_cast<PtrInt>(i), false);
# endif






















































































































