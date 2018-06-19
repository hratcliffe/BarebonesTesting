

#include <stdio.h>
#include "tests.h"

#ifdef USE_MPI
#include <mpi.h>
#endif
//#include "test_min.h"
#include <iomanip>
#include <math.h>
#include <cmath>


#define calc_type double

std::vector<double> cubic_solve(calc_type an, calc_type bn, calc_type cn);

namespace testbed_example{

mpi_info_struc setup_MPI(int argc, char ** argv);
void example_testing(testbed::tests * mytestbed);

testbed::USER_ERR my_err = testbed::add_err("My error!");
//Defining a new error. USER_ERR are constant and cannot be assigned to
//They can be initialised like this
testbed::USER_ERR my_err2 = testbed::add_err("Different error");

class test_entity_sample : public testbed::test_entity{
/** */
  private:
  public:
  test_entity_sample(){
    name = "sample test";
  }
  virtual ~test_entity_sample(){;};
  virtual testbed::TEST_ERR run();

};

testbed::TEST_ERR test_entity_sample::run(){
  testbed::TEST_ERR err=testbed::TEST_PASSED;
  report_info("Checking cubic roots");

  //  x^3 - 17x^2 + 92x - 150.
  std::vector<double> result = cubic_solve(-17.0, 92.0, -150.0);
  if((result.size() != 1) || std::abs(result[0] - 3.0) > testbed::PRECISION) err|=testbed::TEST_WRONG_RESULT;
  //Test with polynomial of known integer coefficients and roots

  result = cubic_solve(-20.5, 100.0, -112.76);
  //test with random polynomial which happens to have 3 real roots
  calc_type res_el, tot;
  for(size_t i=0; i<result.size(); ++i){
    res_el = result[i];
    tot = std::pow(res_el, 3) -20.5*std::pow(res_el, 2) +100.0*res_el - 112.76;
    if(std::abs(tot) > testbed::PRECISION){
      err|=testbed::TEST_WRONG_RESULT;
      report_info("Cubic root does not solve polynomial, mismatch "+testbed::mk_str(tot)+" for root "+testbed::mk_str(res_el), 2);
      
    }
  }
  if(err == testbed::TEST_PASSED) report_info("Cubic roots OK");

  report_err(err);
  return err;

}

REGISTER(sample);

class test_entity_second : public testbed::test_entity{
/** */

  private:
  public:
  test_entity_second(){
    name = "second";
  }
  virtual ~test_entity_second(){;};
  virtual testbed::TEST_ERR run();
  int number;
  void setup(int a){number=a;}
  void setup(int a, std::string b){number=a;}
  void setup(){name="still second";}

};
testbed::TEST_ERR test_entity_second::run(){
  report_info("Number is "+testbed::mk_str(number));
  report_err(testbed::TEST_PASSED);
  return testbed::TEST_PASSED;
}
REGISTER(second);

class test_entity_fail : public testbed::test_entity{
/** */

  private:
  public:
  test_entity_fail(){
    name = "fails";
  }
  virtual ~test_entity_fail(){;};
  virtual testbed::TEST_ERR run();
};
testbed::TEST_ERR test_entity_fail::run(){
/**\brief All sort of error code examples! 
*
* Report_err and report_info determine output. NB report_err prints an error code when the test is run. The return value of this function is used to add up the number of failing tests for the final report. Errors are combined with | (pipe, or). Info can be conditionally reported, and/or reported according to the current level of output verbosity. 
*/
  testbed::TEST_ERR err = testbed::TEST_PASSED;
  err |= testbed::TEST_WRONG_RESULT;
  report_info("This is always reported", 0);
  if(err != testbed::TEST_PASSED) report_info("This conditionally reported", 2);

  report_err(testbed::TEST_WRONG_RESULT | my_err);
  report_err(my_err2);
  return testbed::TEST_WRONG_RESULT | my_err;
}
REGISTER(fail);

class test_entity_setup : public testbed::test_entity{
/** */

  private:
  public:
  test_entity_setup(){
    name = "setup";
  }
  virtual ~test_entity_setup(){;};
  virtual testbed::TEST_ERR run();
  bool isset = false;
  void setup(){isset=true;}
};
testbed::TEST_ERR test_entity_setup::run(){
  report_info("Isset is "+testbed::mk_str(isset));
  report_err(testbed::TEST_PASSED);
  return testbed::TEST_PASSED;
}
REGISTER(setup);

class test_entity_setup2 : public testbed::test_entity{
/** */

  private:
  public:
  test_entity_setup2(){
    name = "setup2";
  }
  virtual ~test_entity_setup2(){;};
  virtual testbed::TEST_ERR run();
  bool isset = false;
  void setup(bool set){isset=set;}
};
testbed::TEST_ERR test_entity_setup2::run(){
  report_info("Isset is "+testbed::mk_str(isset));
  report_err(testbed::TEST_PASSED);
  return testbed::TEST_PASSED;
}
REGISTER(setup2);
}

int main(int argc, char ** argv){

  testbed::tests * mytestbed = new testbed::tests();

#ifdef USE_MPI
  mpi_info_struc mpi_info = setup_MPI(argc, argv);
  testbed::set_mpi(mpi_info);
#endif

  testbed_example::example_testing(mytestbed);
  
  delete mytestbed;

#ifdef USE_MPI
  MPI_Finalize();
#endif
}

mpi_info_struc setup_MPI(int argc, char ** argv){
/** \brief Example of MPI setup
*
* Creates an mpi_info_struc and sets n_procs and rank according to MPI library functions
*/
#ifdef USE_MPI

  int ierr, rank, n_procs;
  ierr = MPI_Init(&argc, &argv);
  //Note any other command line arg processing should account for this...

  ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  ierr = MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
  
  mpi_info_struc mpi_info;
  mpi_info.rank = rank;
  mpi_info.n_procs = n_procs;

  return mpi_info;
#else
  return mpi_info_null;
#endif

}

namespace testbed_example{

void example_testing(testbed::tests * mytestbed){
  testbed::set_filename("testing.log");
  testbed::set_colour("fail", 'm');

  mytestbed->setup_tests();

  //Adding simple tests:
  mytestbed->add("sample");
  mytestbed->add("fail");

  //Adding a test with an argument-less setup function, with and without invoking it
  mytestbed->add("setup");
  ADDABLE_FN_TYPE(setup) mysetupfun = ADDABLE_FN_NOARG(setup::setup);
  mytestbed->add("setup", mysetupfun);

  //Adding a test with a single argument setup function, in one line. NB the type cast
  mytestbed->add("setup2", (ADDABLE_FN_TYPE(setup2)) ADDABLE_FN(setup2::setup, true));

  //Adding several versions of a test, with an overloaded setup function
  RESOLVED_FN_TYPE(second, tmpfn)(int) = RESOLVED_FN(second, setup);
  ADDABLE_FN_TYPE(second) myfun = MEMBER_BIND(tmpfn, 1);
  mytestbed->add("second", myfun);

  RESOLVED_FN_TYPE(second, tmpfn2)(int, std::string) = RESOLVED_FN(second, setup);
  myfun = MEMBER_BIND(tmpfn2, 5, "bum");
  mytestbed->add("second", myfun);

  RESOLVED_FN_TYPE(second, tmpfn3)(void) = RESOLVED_FN(second, setup);
  myfun = MEMBER_BIND_NOARG(tmpfn3);
  mytestbed->add("second", myfun);

  testbed::my_print("Available tests:");
  mytestbed->print_available();

  testbed::my_print("Running tests");
  mytestbed->run_tests();

}
/**< Adding tests to the remit
*
*\copydoc dummy_simple
*\copydoc dummy_overload
*
*/

/** @class dummy_simple
*For a test where all necessary setup can be done in the constructor, simply call mytestbed->add("name"); where name is the name the class is registered under. 
*For a simple setup function, with any number of parameters but no overloads, we bind together the setup function arguments to the values required, and pass that as second argument to add(). Using supplied macros: 
ADDABLE_FN_TYPE(name) myfun = ADDABLE_FN(name::functionname, 1, "hello"); etc
and then mytestbed->add("name", myfun)
Before the test is run, the setup function will be invoked with the arguments given. Multiple calls to add with different arguments may be made, each adding a new test object
*
*The macros above expand to std::function<void(test_entity_name *)> myfun = std::bind(&test_entity_name::functionname, std::placeholders::_1, 1, "hello") Note the placeholder for the this argument as we're using a member function. The end result of the binding which is passed to add must be a function taking exactly one parameter, a this pointer for the object specified by "name". 

 @class dummy_overload
For overloaded setup functions, there is an additional step. std::bind, which underlies the ADDABLE_FN macro, has to be told which overload is required. 
So we first do void (test_entity_second::*tmpfn)(int) = &test_entity_second::setup_member;
to create tmpfn with the correct signature, and then bind to the parameters of tmpfn like this myfun = std::bind(tmpfn, std::placeholders::_1, 1); Finally we call add("name", myfun) as before. Macros to build this syntax are used like this:   RESOLVED_FN_TYPE(name, tmpfunctionname)(argument specification) = RESOLVED_FN(name, functionname); myfun = MEMBER_BIND[_NOARG](tmpfunctionname [, arg list]);

*/
}
std::vector<double> cubic_solve(calc_type an, calc_type bn, calc_type cn){
/** \brief Finds roots of cubic x^3 + an x^2 + bn x + cn = 0
*
* Uses Num. Rec. equations, which are optimised for precision. Note that if x >>1 precision errors may result. Returns real solutions only
*/

  calc_type Q, R, bigA, bigB, Q3, R2, bigTheta;
  std::vector<calc_type> ret_vec;
  const double pi = 3.14159265359;/**< Pi */

  Q = (std::pow(an, 2) - 3.0 * bn)/9.0;
  R = (2.0* std::pow(an, 3) - 9.0 * an *bn + 27.0*cn)/54.0;
  
  R2 = std::pow(R, 2);
  Q3 = std::pow(Q, 3);
  
  if( R2 < Q3){
    
    bigTheta = std::acos(R/sqrt(Q3));
    calc_type minus2sqrtQ = -2.0*std::sqrt(Q);
    
    ret_vec.push_back(minus2sqrtQ*std::cos(bigTheta/3.0) - an/3.0);
    ret_vec.push_back(minus2sqrtQ*std::cos((bigTheta + 2.0*pi)/3.0) - an/3.0);
    ret_vec.push_back(minus2sqrtQ*std::cos((bigTheta - 2.0*pi)/3.0) - an/3.0);

  }else{
    calc_type ret_root;
    int sgn = R !=0 ? (long double) R/(long double) std::abs(R) : 0;
    bigA = - sgn*std::pow((std::abs(R) + std::sqrt(R2 - Q3)), 1.0/3.0 );

    (bigA != 0.0) ? (bigB = Q / bigA) : (bigB = 0.0);
    ret_root = (bigA + bigB) - an/3.0;

    ret_vec.push_back(ret_root);
  }
  return ret_vec;

}
