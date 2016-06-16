//
//  tests.h
//  
//
//  Created by Heather Ratcliffe on 10/11/2015.
// Modified 15/6/2016
//
//
#ifdef RUN_TESTS_AND_EXIT

#ifndef _tests_h
#define _tests_h

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <map>

#define calc_type double

#define REGISTER(x) static testbed::Registrar<test_entity_ ## x> registrar_ ## x( # x)
//static testbed::Registrar<test_entity_sample> registrar("sample");
//This string can be anything, but for sanity, make it the additional part of the test entity
//static testbed::Registrar<test_entity_sample> registrar_sample;
//When this breaks, google "most vexing parse"


struct mpi_info_struc{
  int rank;
  int n_procs;
  bool operator ==(const mpi_info_struc& rhs) const { return rhs.rank == this->rank && rhs.n_procs == this->n_procs;}
};

const struct mpi_info_struc mpi_info_null = {0, 0};

namespace testbed{

/*#ifdef USEMPI
  const mpi_info_struc mpi_info={1, 2};
#else*/
  mpi_info_struc mpi_info = mpi_info_null;
//#endif

  const int TEST_PASSED = 0;
  const int TEST_WRONG_RESULT = 1;
  const int TEST_NULL_RESULT = 2;
  const int TEST_ASSERT_FAIL = 4;
  const int TEST_USERDEF_ERR1 = 8;
  const int TEST_USERDEF_ERR2 = 16;
  const int TEST_USERDEF_ERR3 = 32;
  const int TEST_USERDEF_ERR4 = 64;
  const int err_tot = 8;
  const calc_type PRECISION = 1e-10;/**< Constant for equality at normal precision i.e. from rounding errors etc*/
  const calc_type NUM_PRECISION = 1e-6;/**< Constant for equality at good numerical precision, e.g. from numerical integration over 100-1000 pts*/
  const calc_type LOW_PRECISION = 5e-3;/**< Constant for equality at low precision, i.e. different approximations to an expression*/
  const int max_verbos = 4;
  std::string filename = "tests.log";/**<Test log file*/
  bool hasColour = false;

  const int err_codes[err_tot] ={TEST_PASSED, TEST_WRONG_RESULT, TEST_NULL_RESULT, TEST_ASSERT_FAIL, TEST_USERDEF_ERR1, TEST_USERDEF_ERR2, TEST_USERDEF_ERR3, TEST_USERDEF_ERR4};/**< List of error codes available*/

  std::string err_names[err_tot]={"None", "Wrong result", "Invalid Null result", "Assignment or assertion failed", "", "", "", ""};/**< Names corresponding to error codes, which are reported in log files*/

  inline void set_filename(std::string name){filename = name;}
  inline void set_mpi(mpi_info_struc mpi_info_in){mpi_info=mpi_info_in;}

  //Wrapper functions ensuring only one MPI rank writes
  inline void my_print(std::string text, int rank, int rank_to_write=0, bool noreturn=false){
  /** \brief Write output
  *
  * Currently dump to term. Perhaps also to log file. Accomodates MPI also. Set rank_to_write to -1 to dump from all. Default value is 0
  */
    if(rank == rank_to_write || rank_to_write == -1){
      std::cout<< text;
      if(!noreturn) std::cout<<std::endl;

    }

  }
  inline void my_print(std::fstream * handle, std::string text, int rank, int rank_to_write=0, bool noreturn=false){
  /** \brief Write output
  *
  * Currently dump to term. Perhaps also to log file. Accomodates MPI also. Set rank_to_write to -1 to dump from all. Default value is 0
  */
    if((rank == rank_to_write || rank_to_write == -1) && handle!=nullptr){
      *handle<<text;
      if(!noreturn) *handle<<std::endl;

    }else if(rank == rank_to_write || rank_to_write == -1){
      std::cout<<text;
      if(!noreturn) std::cout<<std::endl;


    }

  }
  inline void my_print(std::string text, bool noreturn=0){
      std::cout<< text;
      if(!noreturn) std::cout<<std::endl;
}
  inline void my_print(std::fstream * handle, std::string text){
    if(handle!=nullptr){
      *handle<<text<<std::endl;
    }else{
      std::cout<<text<<std::endl;
    }

  }

   void trim_string(std::string &str, char ch=' '); /**< Trim all leading/trailing ch's from str*/

#ifdef USECPP11
//We have to_string, just wrap that for numeric types

  template <typename T> std::string mk_str(T input){
    return std::to_string(input);
  }

  template <typename T> std::string mk_str(T input, bool noexp){
    return std::to_string(input);
  }

#else
  inline std::string mk_str(int i){

    char buffer[25];
    std::sprintf(buffer, "%i", i);
    std::string ret = buffer;
    return ret;
    
  }
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
  inline std::string mk_str(long double i, bool noexp){return mk_str((double) i, noexp);};
  inline std::string mk_str(float i, bool noexp){return mk_str((double) i, noexp);};

#endif

  inline void check_term(){
    /** \brief Check terminal capabilites
    *
    *Checks terminal reporting and automatically sets Apple terminal to 8 colour as it wrongly reports 256-support.
    */
    std::string term_env;

    if(getenv("TERM")!=NULL) term_env = getenv("TERM");
    if(term_env == "xterm-256color" || "xterm-8color") hasColour = true;

  }


  /**\brief Testing instance
  *
  *Consists of at least a constructor doing any setup required, a name string for output id, a function run() taking no parameters which performs the necessary test and a destructor doing cleanup.
  */
  class tests;
  class test_entity{
  private:
  public:

    tests * parent;
    std::string name;/**< The name of the test, which will be reported in the log file*/
    test_entity(){parent = nullptr; name = "";}
    virtual ~test_entity(){;}
    virtual int run()=0;/*Pure virtual because we don't want an instances of this template*/
    void report_info(std::string info, int verb_to_print =1);
    void report_err(int err);

  };

  /* The factory implmentation is adapted from http://www.codeproject.com/Articles/567242/AplusC-b-bplusObjectplusFactory*/
  class test_factory{
  friend class tests;
  private:
    std::map<std::string, std::function<test_entity*(void)> > factoryFunctionRegistry;
  public:

    void registerFactoryFunction(std::string name, std::function<test_entity*(void)> classFactoryFunction){ factoryFunctionRegistry[name] = classFactoryFunction;}
    static test_factory * instance();
    std::shared_ptr<test_entity> create(std::string name);

  };
  test_factory * test_factory::instance(){
    static test_factory factory;
    return &factory;
  }

  std::shared_ptr<test_entity> test_factory::create(std::string name){
      test_entity * instance = nullptr;
//      for(auto it = this->factoryFunctionRegistry.begin(); it !=this->factoryFunctionRegistry.end(); it++) std::cout<<it->first;
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
  public:
      Registrar(std::string name)
      {
          // register the class factory function
//          std::cout<<"here"<<'\n';
          test_factory::instance()->registerFactoryFunction(name,
                  [](void) -> test_entity * { return new T();});
      }
  };

  /**\brief Test controller
  *
  *Controls running of tests and their logging etc
  *To add a test, do the following:
  *Descend an object from test_entity which has at least a constructor doing any setup required, a name string for output id, a function run taking no parameters which performs the necessary test and a destructor doing cleanup. Add any other member variables or functions required, including their headers also. In tests::setup_tests create an instance of your class as test_obj = new your_class() and then add your test to the remit using add_test(test_obj); Alternately make the instance and use the global test_bed using test_bed->add(your pntr) from anywhere.
  *To add errors, add the message into the blank spaces in the list below, err_names, and declare a const int of desired name aliased to TEST_USERDEF_ERR* where * = 1-4
  *To report the errors by code, call test_bed->report_err(err); To report other salient information use test_bed->report_info(info, verbosity) where the second parameter is an integer describing the verbosity setting at which to print this info (0=always, the larger int means more and more detail).
  */

  class tests{
  private:

  /** \brief Make an error message
  *
  * Converts error code to printable string, adds code for reference and adds test name. Note code is bitmask and additional errors are appended together
  */
    std::string get_printable_error(int err, int test_id){
      std::string err_string="";
      if(err!=TEST_PASSED){
        for(int i=err_tot-1; i>0; --i){
          //Run most to least significant
          if((err & err_codes[i]) == err_codes[i]){
            err_string +=err_names[i] + ", ";
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

  /*  template <typename T> void configure_test(std::string name, std::function<void(T)> myfunc){
      //apply a setup function
      //As a test, create a name and call the function
      std::shared_ptr<test_entity> eg = testbed::test_factory::instance()->create(name);
      //<dynamic_cast eg->myfunc;
//      test_entity * this_is_terrible = eg.get();
      T oh_gods_the_humanity = dynamic_cast<T> (eg.get());
//      myfunc(dynamic_cast<test_entity *> (eg) );
      myfunc(oh_gods_the_humanity);

    }*/

    template <typename T> void add(std::string name, std::function<void(T)> myfunc){
      //apply a setup function
      //As a test, create a name and call the function
      std::shared_ptr<test_entity> eg = testbed::test_factory::instance()->create(name);
      //<dynamic_cast eg->myfunc;
//      test_entity * this_is_terrible = eg.get();
      T oh_gods_the_humanity = dynamic_cast<T> (eg.get());
//      myfunc(dynamic_cast<test_entity *> (eg) );
      myfunc(oh_gods_the_humanity);
      eg->parent = this;
      test_list.push_back(eg);

    }

    /** \brief Log error
    *
    * Logs error text corresponding to code err for test defined by test_id. Errors are always recorded.*/
    void report_err(int err, int test_id=-1){
      if(test_id == -1) test_id = current_test_id;
      if(err ==TEST_PASSED) set_colour('b');
      else set_colour('r');
      my_print(outfile, get_printable_error(err, test_id), mpi_info.rank);
      my_print(nullptr, get_printable_error(err, test_id), mpi_info.rank);
      set_colour();
  }

    /** \brief Log other test info
    *
    *Records string info to the tests.log file and to screen, according to requested verbosity.
    */
    void report_info(std::string info, int verb_to_print = 1, int test_id=-1){
      if(test_id == -1) test_id = current_test_id;
      if(verb_to_print <= this->verbosity){
        my_print(outfile, info, mpi_info.rank);
        my_print(nullptr, info, mpi_info.rank);
      }
  };

    tests(){
      this->verbosity = max_verbos;
      check_term();
    };
    ~tests(){cleanup_tests();};
    void setup_tests(){
      /** \brief Setup test bed
      *
      *Opens reporting file.
      */
      outfile = new std::fstream();
      outfile->open(filename.c_str(), std::ios::out);
      if(!outfile->is_open()){
        my_print("Error opening "+filename, mpi_info.rank);
        //can't log so return with empty test list
        return;
      }
    }

    void add(std::string name){
      std::shared_ptr<test_entity> eg = testbed::test_factory::instance()->create(name);
      eg->parent = this;
      test_list.push_back(eg);
    }
    void print_available(){
    /** Print names of all registered tests */
      auto registry = testbed::test_factory::instance()->factoryFunctionRegistry;
          for(auto it = registry.begin(); it !=registry.end(); it++) std::cout<<it->first<<'\n';
    
    }

    /** Delete test objects */
    void cleanup_tests(){
      if(outfile->is_open()){
        this->report_info("Testing complete and logged in " +filename, 0);
        outfile->close();
      }else{
        this->report_info("No logfile generated", 0);
      }
      delete outfile;
      test_list.clear();
    };

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
        this->set_colour('r');
        this->report_info(mk_str(total_errs)+" failed tests", mpi_info.rank);
        this->set_colour();
      }else{
        this->report_info("All tests passed", mpi_info.rank);
      }
    };

    /** Set the verbosity of testing output, from 0 (minimal) to max_verbos.*/
    void set_verbosity(int verb){if((verb > 0)) this->verbosity = std::max(verb, max_verbos);};

    /** \brief Set output text colour*/
    void set_colour(char col=0){my_print(this->get_color_escape(col), mpi_info.rank, 0, true);};

    std::string get_color_escape(char col=0);
    
  };

  inline void test_entity::report_info(std::string info, int verb_to_print){parent->report_info(info, verb_to_print);};
  inline void test_entity::report_err(int err){parent->report_err(err);};


  //Break this out because it's giant case
  inline std::string tests::get_color_escape(char col){
  /** \brief Get color string
  *
  * Returns the terminal escape string to set given colour
  *Set terminal output colour using std escape sequences. Accepts no argument to return to default, or rgb, cmyk and white to test text colour. NB technically not MPI safe. Use sparingly to highlight important information.

  */
    if(!hasColour) return "";
    
    switch (col) {
      case 0:
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

      default:
        return "";
    }

  }

};

#endif
#endif