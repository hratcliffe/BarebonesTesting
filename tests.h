//
//  tests.h
//  
//
//  Created by Heather Ratcliffe on 10/11/2015.
// Modified 15/6/2016
// Updated 18/6/2018
//
namespace testbed{
//Doxygen inside namespace to shorten Doxygen name resolution
/*! \mainpage notitle
\section Purp Purpose
This is a very small testing rig for semi-automatic, non-specific testing. Test objects are created by name and can then be run in a block. Error codes, MPI aware error logging and some ANSI compatible colourising routines are provided. 
\section basic Basic useage
Outline code is available in main.cpp Start by creating a tests object and setting-up the logfile:

  testbed::tests * mytestbed = new testbed::tests();

  testbed::set_filename("mylog.log");

  mytestbed->\ref tests::setup_tests "setup_tests()";

Add some tests to run:

  mytestbed->\ref tests::add "add(\"sample\")";

  mytestbed->\ref tests::add "add(\"fail\")";

Run them and clean up:

  mytestbed->\ref tests::run_tests "run_tests()";

  delete mytestbed;

mylog.log now contains all test results and additional information. See the examples in testbed_example::example_testing() for more.

\section Log Output
Simple output control is provided via \ref test_entity::report_info "report_info" and \ref test_entity::report_err "report_err" and also via the direct my_print() functions.
\subsection MPI MPI
Very basic MPI support is provided. Logging can be done by all ranks, or by only one, rank 0 by default. To enable this, create a testbed::mpi_info_struc and set the two fields, rank and n_procs. See ::testbed_example::setup_MPI().

\subsection Colour Colour
For ANSI compatible terminals we can colourise output. set_colour() allows to set any of the standard 8 colour set, RGB, CMYK plus white. A few attributes such as bold are also supported. See tests::set_colour().

\subsection Verb Verbosity
Error messages are always printed but supplementary information via test_entity::report_info() can be given a verbosity level. There are n levels from 0 (minimal) to testbed::max_verbos (maximal). This is 4 by default but can be changed by editing file. The working level of verbosity is set via tests::set_verbosity between 0 and testbed::max_verbos. See ::testbed_example::test_entity_fail::run() for examples.

\section Test Writing a test
An example test is given in ::testbed_example::test_entity_sample Here we solve some cubics and do basic error reporting etc. Note we include math header to use this. We can explicitly allocate memory in the test constructor, and free it in the destructor, or do both of these in the run method. run() may call other local or external methods etc.

\section Setup Using a setup function
Running the same test using different parameters is acheived by adding the test along with a setup function. An example without arguments is given in test_example::test_entity_setup and one with in test_example::test_entity_setup2. We use some macros (see \ref Macros) to bind the arguments with the setup function and pass this as well as the function name to testbed::tests::add. 
\copydoc dummy_simple
See also testbed_example::example_testing().


\subsection Overload What if my setup function is overloaded
If the setup function has overloads with different number/type of arguments, there is slightly more complicated procedure, to resolve the correct overload when we add arguments. 
\copydoc dummy_overload
See also testbed_example::example_testing().

\section Macros What are all these macros doing?
The previous section involves using several macros. These are a shortcut to writing out the syntax, and are NOT nest-safe. A makefile recipe, preprocess, is given to expand these by preprocessing JUST the relevant file and the tests.h header. Alternately, use the expanded syntax directly. 


@date 18/Jun 2016 @author H Ratcliffe
 */
}

#ifndef _tests_h
#define _tests_h

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <map>

#define PASTE(x, y) x ## y
#define REGISTER(x) static testbed::Registrar<test_entity_ ## x> registrar_ ## x( # x)
/**<Expands out the correct syntax for registering function with testbed*/
//When this breaks, google "most vexing parse"


#define ADDABLE_FN_TYPE(x) std::function<void(test_entity_ ## x *)>
#define ADDABLE_FN_NOARG(x) std::bind(&test_entity_ ##x, std::placeholders::_1)
#define ADDABLE_FN(x, ...) std::bind(&test_entity_ ##x, std::placeholders::_1, __VA_ARGS__)
/**<These expand out the correct syntax for binding arguments to a setup function*/

#define RESOLVED_FN_TYPE(x, y) void (test_entity_ ##x ::* y)
#define RESOLVED_FN(x, y) &test_entity_##x :: y
#define MEMBER_BIND(x, ...) std::bind( x ,std::placeholders::_1, __VA_ARGS__)
#define MEMBER_BIND_NOARG(x) std::bind( x ,std::placeholders::_1)
/**<These expand out the syntax for resolving overloads and binding arguments*/

struct mpi_info_struc{
  int rank;
  int n_procs;
  bool operator ==(const mpi_info_struc& rhs) const { return rhs.rank == this->rank && rhs.n_procs == this->n_procs;}
};
/**< Structure holding minimal mpi info*/

const struct mpi_info_struc mpi_info_null = {0, 0}; /**< \internal Null MPI struct */

namespace testbed{
/** Namespace for all the testbed code*/

  struct colours{
    char fail;
    char info;
    char pass;
    char normal;
    
  };
  
  const int TEST_PASSED = 0;
  const int TEST_WRONG_RESULT = 1;
  const int TEST_NULL_RESULT = 2;
  const int TEST_ASSERT_FAIL = 4;
  const int TEST_OTHER = 8;
  const int TEST_USER_FAILED = 16;
  const int TEST_USERDEF_ERR1 = 32;
  const int TEST_USERDEF_ERR2 = 64;
  const int TEST_USERDEF_ERR3 = 128;
  const int TEST_USERDEF_ERR4 = 256;
  const int max_err = 10;
  /* Error codes list */

  const double PRECISION = 1e-10;/**< Constant for equality at normal precision i.e. from rounding errors etc*/
  const double NUM_PRECISION = 1e-6;/**< Constant for equality at good numerical precision, e.g. from numerical integration over 100-1000 pts*/
  const double LOW_PRECISION = 5e-3;/**< Constant for equality at low precision, i.e. different approximations to an expression*/
  const int max_verbos = 4;

  typedef int TEST_ERR;/**< Type for error codes*/
  typedef const int USER_ERR; /**<Special type for defining a new error code */

  const int err_codes[max_err] ={TEST_PASSED, TEST_WRONG_RESULT, TEST_NULL_RESULT, TEST_ASSERT_FAIL, TEST_OTHER, TEST_USER_FAILED, TEST_USERDEF_ERR1, TEST_USERDEF_ERR2, TEST_USERDEF_ERR3, TEST_USERDEF_ERR4};/**< List of error codes available*/

  class config{
    public:
      /**< \internal Colours for printing according to function*/
      
      struct colours test_colours={'R', 'g', 'b', '0'};
      /**< \internal Default colours */

      mpi_info_struc mpi_info = mpi_info_null;
      /**< \internal Default MPI struct. This is null, i.e. does not distinguish between processors*/
      
      std::string filename = "tests.log";/**<Default test log file*/
      bool hasColour = false;/**< \internal Flag for terminal colour use*/
      int last_err = 6;
      std::string err_names[max_err]={"None", "Wrong result", "Invalid Null result", "Assignment or assertion failed", "Other error", "Failed to allocate errorcode", "", "", "", ""};/**< Names corresponding to error codes, which are reported in log files*/
      static config * instance(){static config inst; return &inst;}
  };

  inline USER_ERR add_err(const std::string text){
    /** \brief Add a user-defined error message
    *
    *Adds an error message to the defined set. There is a limited number (currently 4) of additional codes. @param text The printable message associated with this error @return The new error code, or if the maximum has been reached, a TEST_USER_FAILED error.
    */
    if(config::instance()->last_err >= max_err-1) return TEST_USER_FAILED;
    config::instance()->err_names[config::instance()->last_err] = text;
    config::instance()->last_err ++;
    return err_codes[config::instance()->last_err - 1];
  }
  
  inline void set_filename(std::string name){config::instance()->filename = name;}/**< Set the output filename. Default value is "tests.log". */
  inline void set_mpi(mpi_info_struc mpi_info_in){config::instance()->mpi_info=mpi_info_in;}
  /**< \brief Setup MPI
  *
  * Logging and print functions by default print only on the 0 ranked processor. Use this function to set the mpi_info for each processor. A testbed::mpi_info_struc has two fields, n_procs for the total number of processors, and rank, for each processor's rank.
  */

  inline void set_colour(std::string function, char colour){
    /** \brief Set colours used
    *
    * Sets the output colour for given useage to colour. Applies only if terminal supports colour. Functions are: pass, fail, info, normal used to report test passes, test failures, additional info via report_info and the normal mode. Available colours are rgb, cmyk and white.
    */

    if(function == "fail"){
      config::instance()->test_colours.fail = colour;
    }else if(function == "info"){
      config::instance()->test_colours.info = colour;
    }else if(function == "pass"){
      config::instance()->test_colours.pass = colour;
    }else if(function == "normal"){
      config::instance()->test_colours.normal = colour;
    }else{
    }
  }

  inline void my_print(std::string text, int rank_to_write=0, int rank=config::instance()->mpi_info.rank, bool noreturn=false){
  /** \brief Write output
  *
  *MPI aware writing routine. Writes string text to stdout, only from processor with rank equal rank_to_write. This defaults to 0. If mpi_info has not been set, via set_mpi, ALL processors will print, in unspecified order.
  */
    if(rank == rank_to_write || rank_to_write == -1){
      std::cout<< text;
      if(!noreturn) std::cout<<std::endl;

    }

  }
  inline void my_print(std::fstream * handle, std::string text, int rank_to_write=0, int rank=config::instance()->mpi_info.rank, bool noreturn=false){
  /** \brief Write output
  *
  *MPI aware writing routine. Writes string text to given file, only from processor with rank equal rank_to_write. This defaults to 0. If mpi_info has not been set, via set_mpi, ALL processors will print, in unspecified order.
  */
    if((rank == rank_to_write || rank_to_write == -1) && handle!=nullptr){
      *handle<<text;
      if(!noreturn) *handle<<std::endl;

    }else if(rank == rank_to_write || rank_to_write == -1){
      std::cout<<text;
      if(!noreturn) std::cout<<std::endl;
    }
  }

  template <typename T> std::string mk_str(T input){
    return std::to_string(input);
  }
  /* Wrap C++11 to_string to be consistent with our overloaded mk_str*/

  /**< Make string from input. Note has overloads where one can specify noexp=true to prevent using scientific form output*/

  inline std::string mk_str(double i, bool noexp=0){
    char buffer[25];
    if(noexp) std::snprintf(buffer, 25, "%f", i);
    else std::snprintf(buffer, 25, "%e", i);
    std::string ret = buffer;
    return ret;
    
  }
  inline std::string mk_str(bool b){
    if(b) return "1";
    else return "0";

  }
  inline std::string mk_str(long double i, bool noexp){return mk_str((double) i, noexp);}
  inline std::string mk_str(float i, bool noexp){return mk_str((double) i, noexp);}
  /* Some overloads to mk_str for non-scientific output, and for bool type*/


  inline void check_term(){
    /** \brief Check terminal capabilites
    *
    *Checks terminal reporting and automatically sets Apple terminal to 8 colour as it wrongly reports 256-support.
    */
    std::string term_env;

    if(getenv("TERM")!=NULL) term_env = getenv("TERM");
    if(term_env == "xterm-256color" || "xterm-8color") config::instance()->hasColour = true;

  }


  class tests;

  /**\brief Testing instance
  *
  *Consists of at least a constructor doing any setup required, a name string for output id, a function run() taking no parameters which performs the necessary test and a destructor doing cleanup. Ant additional methods may be included. In particular, setup methods with any signature can be run when adding tests. See tests:add
  */
  class test_entity{
  private:
  public:

    tests * parent;/** \internal Parent tests object, for error reporting etc */
    std::string name;/**< The name of the test, which will be reported in the log file*/
    test_entity(){parent = nullptr; name = "";}
    virtual ~test_entity(){;}
    virtual TEST_ERR run()=0;/**< Run method must have this signature. \internal Pure virtual because we don't want an instances of this template*/
    void report_info(std::string info, int verb_to_print =1);
    void report_err(TEST_ERR err);

  };

  class test_factory{
  /** \internal \brief Factory producing test instances
  *
  *Adapted from http://www.codeproject.com/Articles/567242/AplusC-b-bplusObjectplusFactory
  */
  
  friend class tests;
  private:
    std::map<std::string, std::function<test_entity*(void)> > factoryFunctionRegistry;
  public:
    /** \internal Register a test_entity constructor*/
    void registerFactoryFunction(std::string name, std::function<test_entity*(void)> classFactoryFunction){ factoryFunctionRegistry[name] = classFactoryFunction;}
    static test_factory * instance();
    std::shared_ptr<test_entity> create(std::string name);

  };
  inline test_factory * test_factory::instance(){
    /** \internal Return the factory singleton instance*/
    static test_factory factory;
    return &factory;
  }

  inline std::shared_ptr<test_entity> test_factory::create(std::string name){
    /** \internal \brief Create test_entity
    *
    * Create an instance of a test_entity previously registered
    */
      test_entity * instance = nullptr;
      // find name in the registry and call factory method.
      auto it = this->factoryFunctionRegistry.find(name);
      if(it != this->factoryFunctionRegistry.end())
          instance = it->second();
      // wrap instance in a shared ptr and return
      if(instance != nullptr)
          return std::shared_ptr<test_entity>(instance);
      else
          return nullptr;
  }
  
  template<class T>
  class Registrar {
  /** \internal Registrar calls register method of test_factory to do the actual registering. @see REGISTER
  */
  public:
      Registrar(std::string name)
      {
          // register the class factory function
          test_factory::instance()->registerFactoryFunction(name,
                  [](void) -> test_entity * { return new T();});
      }
  };

  /**\brief Test controller
  *
  *Controls running of tests and their logging etc
  */

  /*
  *To add a test, do the following:
  *Descend an object from test_entity which has at least a constructor doing any setup required, a name string for output id, a function run taking no parameters which performs the necessary test and a destructor doing cleanup. Add any other member variables or functions required, including their headers also.
  In tests::setup_tests create an instance of your class as test_obj = new your_class() and then add your test to the remit using add_test(test_obj); Alternately make the instance and use the global test_bed using test_bed->add(your pntr) from anywhere.
  *To add errors, add the message into the blank spaces in the list below, err_names, and declare a const int of desired name aliased to TEST_USERDEF_ERR* where * = 1-4
  *To report the errors by code, call test_bed->report_err(err); To report other salient information use test_bed->report_info(info, verbosity) where the second parameter is an integer describing the verbosity setting at which to print this info (0=always, the larger int means more and more detail).
  */

  class tests{
  private:

  /** \brief Make an error message
  *
  * Converts error code to printable string, adds code for reference and adds test name. Note code is bitmask and additional errors are appended together
  */
    std::string get_printable_error(TEST_ERR err, int test_id){
      std::string err_string="";
      if(err!=TEST_PASSED){
        for(int i=max_err-1; i>0; --i){
          //Run most to least significant
          if((err & err_codes[i]) == err_codes[i]){
            err_string +=config::instance()->err_names[i] + ", ";
          }
        }
        err_string = "Error "+err_string+"(code "+mk_str(err)+") on";
      }
      else err_string = "Passed";
      return err_string+" test "+test_list[test_id]->name;
  }

    std::fstream * outfile; /**< Output file handle*/
    int current_test_id;/**< Number in list of test being run*/
    std::vector<std::shared_ptr<test_entity> > test_list;/**< List of tests to run*/
    int verbosity;/**< Verbosity level of output*/
  public:

    template <typename T> void add(std::string name, std::function<void(T)> myfunc){
    /** \brief Add test to remit
    *
    *Adds a previously registered test by name, calling the given setup function in the process.
    */
      std::shared_ptr<test_entity> eg = testbed::test_factory::instance()->create(name);
      if(eg){
        //Unpack raw pointer and invoke myfunc on eg
        T test_instance = dynamic_cast<T> (eg.get());
        myfunc(test_instance);
        eg->parent = this;
        //Add now set-up test to list
        test_list.push_back(eg);
      }else{
        my_print("No test "+name);
      }
    }
    void add(std::string name){
    /** \brief Add test to remit
    *
    *Adds a previously registered test by name
    */
      std::shared_ptr<test_entity> eg = testbed::test_factory::instance()->create(name);
      if(eg){
        eg->parent = this;
        test_list.push_back(eg);
      }else{
        my_print("No test "+name);
      }
    }

    /** \brief Log error
    *
    * Logs error text corresponding to code err for test defined by test_id. Errors are always recorded.*/
    void report_err(TEST_ERR err, int test_id=-1){
      if(test_id == -1) test_id = current_test_id;
      if(err ==TEST_PASSED) set_colour(config::instance()->test_colours.pass);
      else set_colour(config::instance()->test_colours.fail);

      my_print(outfile, get_printable_error(err, test_id), 0, config::instance()->mpi_info.rank);
      my_print(nullptr, get_printable_error(err, test_id), 0, config::instance()->mpi_info.rank);
      set_colour();
    }

    /** \brief Log other test info
    *
    *Records string info to the tests.log file and to screen, according to requested verbosity. @param info The text to report @param verb_to_print verbosity level at which to print this info @param test_id
    */
    void report_info(std::string info, int verb_to_print = 1, int test_id=-1){
      set_colour(config::instance()->test_colours.info);
      if(test_id == -1) test_id = current_test_id;
      if(verb_to_print <= this->verbosity){
        my_print(outfile, info, 0, config::instance()->mpi_info.rank);
        my_print(nullptr, info, 0, config::instance()->mpi_info.rank);
      }
      set_colour();
    }

    tests(){
      this->verbosity = max_verbos;
      check_term();
    }
    ~tests(){cleanup_tests();}
    void setup_tests(){
      /** \brief Setup test bed
      *
      *Opens reporting file.
      */
      outfile = new std::fstream();
      outfile->open(config::instance()->filename.c_str(), std::ios::out);
      if(!outfile->is_open()){
        my_print("Error opening "+config::instance()->filename, 0, config::instance()->mpi_info.rank);
        //can't log so return with empty test list
        return;
      }
    }

    void print_available(){
    /** Print names of all registered tests */
      auto registry = testbed::test_factory::instance()->factoryFunctionRegistry;
          for(auto it = registry.begin(); it !=registry.end(); it++) std::cout<<it->first<<'\n';
    }

    /** Delete test objects */
    void cleanup_tests(){
      if(outfile && outfile->is_open()){
        my_print("Testing complete and logged in " +config::instance()->filename, 0, config::instance()->mpi_info.rank);
        outfile->close();
      }else{
        set_colour(config::instance()->test_colours.fail);
        my_print("No logfile generated", 0, config::instance()->mpi_info.rank);
        set_colour();
      }
      delete outfile;
      test_list.clear();
    }

    /** \brief Run scheduled tests
    *
    *Runs each test in list and reports total errors found
    */
    void run_tests(){
      int total_errs = 0;
      for(current_test_id=0; current_test_id< (int)test_list.size(); current_test_id++){
        total_errs += (bool) test_list[current_test_id]->run();
        //Add one if is any error returned
      }
      if(total_errs > 0){
        set_colour(config::instance()->test_colours.fail);
        my_print("\xe2\x9c\x97 ", true);
        set_colour('*');
        my_print(mk_str(total_errs)+" failed tests", config::instance()->mpi_info.rank);
      }else{
        set_colour(config::instance()->test_colours.normal);
        my_print("\xe2\x9c\x93 ", true);
        set_colour('*');
        my_print("All tests passed", config::instance()->mpi_info.rank);
      }
      this->set_colour();
    }

    /** Set the verbosity of testing output, from 0 (minimal) to max_verbos. @see report_info*/
    void set_verbosity(int verb){if((verb > 0)) this->verbosity = std::max(verb, max_verbos);}

    /**
     * @class dummy_colour
     * Available colour codes are RGB, CMYK and W(hite) plus * (bold) _ (underlined) ? (blink, irritating and not supported on some terminals) and $ (reverse foreground and background). A call with no arguments, or with 0 or '0' reset to default.
     */

    /** \brief Set output text colour/style
    *
    * \copydoc dummy_colour    */
    void set_colour(char col=0){my_print(this->get_color_escape(col), config::instance()->mpi_info.rank, 0, true);};

    std::string get_color_escape(char col=0);
    
  };

  /** \copydoc tests::report_info */
  inline void test_entity::report_info(std::string info, int verb_to_print){parent->report_info(info, verb_to_print);}
  /** \copydoc tests::report_err */
  inline void test_entity::report_err(int err){parent->report_err(err);}


  //Break this out because it's giant case
  inline std::string tests::get_color_escape(char col){
  /** \brief
  *\copydoc dummy_colour This returns the terminal escape string to set given colour.
  */
    if(!config::instance()->hasColour) return "";
    if(col >='A' and col <='Z') col += 32;
    //ASCII upper to lower
    switch (col) {
      case 0:
      case '0':
        return "\033[0m";
        break;//Redundant but clearer
      case 'r':
        return "\033[31m";
        break;
      case 'g':
        return "\033[32m";
        break;
      case 'b':
        return "\033[34m";
        break;
      case 'c':
        return "\033[36m";
        break;
      case 'm':
        return "\033[35m";
        break;
      case 'y':
        return "\033[33m";
        break;
      case 'w':
        return "\033[37m";
        break;
      case 'k':
        return "\033[30m";
        break;
      case '*':
      //bold
        return "\033[1m";
        break;
      case '_':
      //underline
        return "\033[4m";
        break;
      case '?':
      //blink. very annoying
        return "\033[5m";
        break;
      case '$':
      //reverse fore/back ground
        return "\033[7m";
        break;
      
      default:
        return "";
    }

  }

}

#endif
