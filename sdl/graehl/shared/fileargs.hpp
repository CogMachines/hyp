









#ifndef GRAEHL__SHARED__FILEARGS_HPP
#define GRAEHL__SHARED__FILEARGS_HPP


















































#include <graehl/shared/large_streambuf.hpp>
#include <graehl/shared/null_deleter.hpp>
#include <graehl/shared/stream_util.hpp>
#include <graehl/shared/teestream.hpp>
#include <graehl/shared/null_ostream.hpp>

#include <graehl/shared/size_mega.hpp>
#include <graehl/shared/string_match.hpp>
#include <graehl/shared/program_options.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/config.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <memory>



#ifndef GRAEHL__DEFAULT_IN
#define GRAEHL__DEFAULT_IN std::cin
#endif

#ifndef GRAEHL__DEFAULT_OUT
#define GRAEHL__DEFAULT_OUT std::cout
#endif

#ifndef GRAEHL__DEFAULT_LOG
#define GRAEHL__DEFAULT_LOG std::cerr
#endif


namespace graehl {











































{








}

inline std::string general_options_desc()
{
  return "Options ("+file_arg_usage()+"):";
}


template <class Stream>
struct stream_traits
{
};


template<>
struct stream_traits<std::ifstream>
{
  BOOST_STATIC_CONSTANT(bool, file_only = true);
  BOOST_STATIC_CONSTANT(bool, read = true);

};

template<>
struct stream_traits<std::ofstream>
{
  BOOST_STATIC_CONSTANT(bool, file_only = true);
  BOOST_STATIC_CONSTANT(bool, read = false);

};

template<>
struct stream_traits<std::ostream>
{
  BOOST_STATIC_CONSTANT(bool, file_only = false);
  BOOST_STATIC_CONSTANT(bool, read = false);

};

template<>
struct stream_traits<std::istream>
{
  BOOST_STATIC_CONSTANT(bool, file_only = false);
  BOOST_STATIC_CONSTANT(bool, read = true);

};

template <class S>
inline void set_null_file_arg(boost::shared_ptr<S> &p)
{
  p.reset();
}

inline void set_null_file_arg(boost::shared_ptr<std::ostream> &p)
{
  p.reset(new null_ostream()); //(&the_null_ostream, null_deleter());
}


// copyable because it's a shared ptr to an ostream, and holds shared ptr to a larger buffer used by it (for non-.gz file input/output) - make sure file is flushed before member buffer is destroyed, though!
template <class Stream>
struct file_arg
{

  typedef large_streambuf<> buf_type;



  typedef boost::shared_ptr<Stream> pointer_type;
  pointer_type pointer; // this will get destroyed before buf (constructed after), which may be necessary since we don't require explicit flush.
  bool none;
public:



























































  operator pointer_type() const { return pointer; }

  bool operator==(Stream const& s) const { return get()==&s; }
  bool operator==(file_arg const& s) const { return get()==s.get(); }
  bool operator!=(Stream const& s) const { return get()!=&s; }
  bool operator!=(file_arg const& s) const { return get()!=s.get(); }
  Stream &operator *() const { return *pointer; }
  Stream *get() const
  {
    return pointer.get();
  }
  Stream *operator ->() const { return get(); }

  std::string name;


  {
    return name.c_str();
  }






  void close()
  {
    set_none();
  }

  typedef file_arg<Stream> self_type;






  file_arg() { set_none(); }





  {  set(s, null_allowed, large_buf); }
  void throw_fail(std::string const& filename, std::string const& msg="")
  {
    name = filename;
    throw std::runtime_error("FAILED("+filename+"): "+msg);
  }

  enum { delete_after = 1, no_delete_after = 0 };






  void clear() {


    buf.reset();

  }


  void set(Stream &s, std::string const& filename="", bool destroy = no_delete_after, std::string const& fail_msg="invalid stream")
  {
    clear();
    if (!s)
      throw_fail(filename, fail_msg);
    if (destroy)
      pointer.reset(&s);
    else
      pointer.reset(&s, null_deleter());
    /*  The object pointed to is guaranteed to be deleted when the last shared_ptr pointing to it is destroyed or reset */
    name = filename;
  }

  template <class filestream>
  void set_checked(filestream &fs, std::string const& filename="", bool destroy = no_delete_after, std::string const& fail_msg="invalid stream")
  {
    try {
      set(dynamic_cast<Stream &>(fs), filename, destroy, fail_msg);
    } catch (std::bad_cast &) {
      throw_fail(filename, " was not of the right stream type");
    }
  }
  BOOST_STATIC_CONSTANT(std::size_t, bufsize = 256*1024);

  // warning: if you call with incompatible filestream type ... crash!
  template <class filestream>
  void set_new(std::string const& filename, std::string const& fail_msg="Couldn't open file")
  {
    std::auto_ptr<filestream> f(new filestream(filename.c_str(), std::ios::binary)); // will delete if we have an exception
    set_checked(*f, filename, delete_after, fail_msg);
    f.release(); // w/o delete
  }

  // set_new_buf is nearly a copy of set_new, except that apparently you can't open the file first then set buffer:
  /*
    Yes --- this was what Jens found. So, for the following code

    ifstream in;
    char buffer[1 << 20];
    in.rdbuf()->pubsetbuf(buffer, 1 << 20);
    in.open("bb");
    string str;
    while (getline(in, str)) {   }

    the buffer will be set to be 1M. But if I change it to the following code,

    ifstream in("bb");
    char buffer[1 << 20];
    in.rdbuf()->pubsetbuf(buffer, 1 << 20);
    string str;
    while (getline(in, str)) {   }

    the buffer will be back to 8k.
  */

  template <class filestream>
  void set_new_buf(std::string const& filename, std::string const& fail_msg="Couldn't open file", bool large_buf = true)
  {
    filestream *f = new filestream();
    std::auto_ptr<filestream> fa(f);
    set_checked(*f, filename, delete_after, fail_msg); // exception safety provided by f
    fa.release(); // now owned by smart ptr
    if (large_buf) give_large_buf();


    f->open(filename.c_str(), std::ios::binary | (read ? std::ios::in : (std::ios::out|std::ios::trunc)));
    if (!*f)
      throw_fail(filename, read?"Couldn't open for input.":"Couldn't open for output.");
  }

  void give_large_buf()
  {
    buf.reset(new buf_type(*pointer));
  }

  enum { ALLOW_NULL = 1, NO_NULL = 0 };





  // warning: if you specify the wrong values for read and file_only, you could assign the wrong type of pointer and crash!
  void set(std::string const& s,
           bool null_allowed = ALLOW_NULL, bool large_buf = true
    )
  {
    const bool read = stream_traits<Stream>::read;
    const bool file_only = stream_traits<Stream>::file_only;
    if (s.empty()) {
      throw_fail("<EMPTY FILENAME>","Can't open an empty filename.  Use \"-0\" if you really mean no file");



      if (read) {
        set_checked(GRAEHL__DEFAULT_IN, s);
      } else {
        set_checked(GRAEHL__DEFAULT_OUT, s);
      }

      set_checked(GRAEHL__DEFAULT_LOG, s);

      set_none();
      return;



      set_gzfile(s);








      if (read)
        set_new_buf<std::ifstream>(s, "Couldn't open input file", large_buf);
      else
        set_new_buf<std::ofstream>(s, "Couldn't create output file", large_buf);
    }
    none = false;
  }

  explicit file_arg(Stream &s, std::string const& name) :




  template <class Stream2>
  file_arg(file_arg<Stream2> const& o) :






  void set_none()


  bool is_none() const
  { return none; }

  operator bool() const
  {
    return !is_none();
  }

  bool is_default_in() const {
    return pointer.get() == &GRAEHL__DEFAULT_IN; }

  bool is_default_out() const {
    return pointer.get() == &GRAEHL__DEFAULT_OUT; }

  bool is_default_log() const {
    return pointer.get() == &GRAEHL__DEFAULT_LOG; }

  bool valid() const
  {
    return !is_none() && stream();
  }
  friend
  bool valid(self_type const& f)
  {
    return f.valid();
  }

  Stream &stream() const
  {
    return *pointer;
  }






  template<class O>
  void print(O &o) const { o << name; }
  template <class I>
  void read(I &i)
  {
    std::string name;
    i>>name;
    set(name);
  }
  TO_OSTREAM_PRINT
  FROM_ISTREAM_READ
};

typedef file_arg<std::istream> istream_arg;
typedef file_arg<std::ostream> ostream_arg;
typedef file_arg<std::ifstream> ifstream_arg;
typedef file_arg<std::ofstream> ofstream_arg;

inline
istream_arg stdin_arg()
{

}

inline
ostream_arg stdout_arg()
{

}

inline
ostream_arg stderr_arg()
{

}















typedef boost::shared_ptr<std::istream> Infile;
typedef boost::shared_ptr<std::ostream> Outfile;
typedef boost::shared_ptr<std::ifstream> InDiskfile;
typedef boost::shared_ptr<std::ofstream> OutDiskfile;

static Infile default_in(&GRAEHL__DEFAULT_IN, null_deleter());
static Outfile default_log(&GRAEHL__DEFAULT_LOG, null_deleter());
static Outfile default_out(&GRAEHL__DEFAULT_OUT, null_deleter());
static Infile default_in_none;
static Outfile default_out_none;
static InDiskfile default_in_disk_none;
static OutDiskfile default_out_disk_none;

inline bool is_default_in(const Infile &i) {
  return i.get() == &GRAEHL__DEFAULT_IN; }

inline bool is_default_out(const Outfile &o) {
  return o.get() == &GRAEHL__DEFAULT_OUT; }

inline bool is_default_log(const Outfile &o) {
  return o.get() == &GRAEHL__DEFAULT_LOG; }

inline bool is_none(const Infile &i)
{ return i.get()==NULL; }

inline bool is_none(const Outfile &o)
{ return o.get()==NULL; }

struct tee_file
{
  tee_file() {
    reset_no_tee();
  }

  void reset_no_tee(std::ostream &o = std::cerr)
  {
    teestreamptr.reset();
    teebufptr.reset();
    log_stream=&o;
  }

  /// must call before you get any tee behavior (without, will go to default log = cerr)!
  void set(std::ostream &other_output)
  {
    if (file) {
      teebufptr.reset(
        new graehl::teebuf(file->rdbuf(), other_output.rdbuf()));
      teestreamptr.reset(
        log_stream = new std::ostream(teebufptr.get()));
    } else {
      log_stream=&other_output;
    }
  }
  ostream_arg file; // can set this directly, then call init.  if unset, then don't tee.

  std::ostream &stream() const
  { return *log_stream; }
  operator std::ostream &() const
  { return stream(); }

private:
  std::ostream *log_stream;
  boost::shared_ptr<graehl::teebuf> teebufptr;
  boost::shared_ptr<std::ostream> teestreamptr;
};

template <class Stream>
inline bool valid(boost::shared_ptr<Stream> const& pfile)
{
  return pfile && *pfile;
}

template <class C>
inline void throw_unless_valid(C const& pfile, std::string const& name="file")
{
  if (!valid(pfile))
    throw std::runtime_error(name+" not valid");
}

template <class C>
inline void throw_unless_valid_optional(C const& pfile, std::string const& name="file")
{
  if (pfile && !valid(pfile))
    throw std::runtime_error(name+" not valid");
}

inline Infile infile(const std::string &s)
{
  return istream_arg(s);
}

inline InDiskfile indiskfile(const std::string &s)
{
  return ifstream_arg(s);
}

inline fs::path full_path(const std::string &relative)
{

}

inline bool directory_exists(const fs::path &possible_dir)
{
  return fs::exists(possible_dir) && fs::is_directory(possible_dir);
}

// works on .gz files!
inline size_t count_newlines(const std::string &filename)
{
  Infile i = infile(filename);
  char c;
  size_t n_newlines = 0;
  while (i->get(c)) {
    if (c=='\n')
      ++n_newlines;
  }
  return n_newlines;
}

inline void native_filename_check()
{

}



























// return the absolute filename that would result from "cp source dest" (and write to *dest_exists whether dest exists) - throws error if source is the same as dest
inline std::string output_name_like_cp(const std::string &source, const std::string &dest, bool *dest_exists = NULL)
{
  fs::path full_dest = full_path(dest);
  fs::path full_source = full_path(source);

  if (directory_exists(full_dest))

  if (dest_exists && fs::exists(full_dest))
    *dest_exists = 1;








  if (fs::equivalent(full_dest, full_source))
    throw std::runtime_error("Destination file is same as source!");









}

inline Outfile outfile(const std::string &s)
{
  return ostream_arg(s);
}

inline OutDiskfile outdiskfile(const std::string &s)
{
  return ofstream_arg(s);
}





























} //graehl





namespace boost { namespace program_options {


/* Overload the 'validate' function for boost::shared_ptr<std::istream>. We use shared ptr
   to properly kill the stream when it's no longer used.
*/
inline void validate(boost::any& v,
                     const std::vector<std::string>& values,
                     boost::shared_ptr<std::istream>* target_type, int)
{
  v = boost::any(graehl::infile(graehl::get_single_arg(v, values)));
}

inline void validate(boost::any& v,
                     const std::vector<std::string>& values,
                     boost::shared_ptr<std::ostream>* target_type, int)
{
  v = boost::any(graehl::outfile(graehl::get_single_arg(v, values)));
}

inline void validate(boost::any& v,
                     const std::vector<std::string>& values,
                     boost::shared_ptr<std::ofstream>* target_type, int)
{
  v = boost::any(graehl::outdiskfile(graehl::get_single_arg(v, values)));
}

inline void validate(boost::any& v,
                     const std::vector<std::string>& values,
                     boost::shared_ptr<std::ifstream>* target_type, int)
{
  v = boost::any(graehl::indiskfile(graehl::get_single_arg(v, values)));
}









}

inline void validate(boost::any& v,
                     const std::vector<std::string>& values,



}

inline void validate(boost::any& v,
                     const std::vector<std::string>& values,





















# endif




#endif