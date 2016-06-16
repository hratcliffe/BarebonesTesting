

#include <stdio.h>
#include "tests.h"
#include <mpi.h>
//#include "test_min.h"
#include <iomanip>
#include <math.h>
#include <cmath>

#define calc_type double
std::vector<double> cubic_solve(calc_type an, calc_type bn, calc_type cn);

class test_entity_sample : public testbed::test_entity{
  private:
  public:
  test_entity_sample(){
    name = "sample test";
  }
  virtual ~test_entity_sample(){;};
  virtual int run();

};

int test_entity_sample::run(){
  int err=testbed::TEST_PASSED;
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
  report_err(err);
  return err;

}

REGISTER(sample);

class test_entity_second : public testbed::test_entity{
  private:
  public:
  test_entity_second(){
    name = "second";
  }
  virtual ~test_entity_second(){;};
  virtual int run();
 // void setup_member(int a, int b){std::cout<<a+b<<'\n';number=a+b;}
  int number;
//  void setup_member(int a){std::cout<<a<<'\n';std::cout<<this->name<<'\n';this->name="fish";std::cout<<this->name<<'\n';number=a;}
  void setup(int a, std::string b){number=a;}
  //{std::cout<<a<<" "<<b<<'\n';std::cout<<this->name<<'\n';number=a;}

};
int test_entity_second::run(){
  report_info(testbed::mk_str(number));
  report_err(0);
  return 0;
}
REGISTER(second);

//void setup_second(test_entity_second * inst, int a){std::cout<<a<<'\n';std::cout<<inst->name<<'\n';}
//void setup_second(test_entity_second * inst, int a){std::cout<<a<<'\n';std::cout<<inst->name<<'\n';inst->name="fish";std::cout<<inst->name<<'\n';}

int main(int argc, char ** argv){

  int ierr, rank, n_procs;

  ierr = MPI_Init(&argc, &argv);
  //Note any other command line arg processing should account for this...

  ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  ierr = MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
  
  mpi_info_struc mpi_info;
  mpi_info.rank = rank;
  mpi_info.n_procs = n_procs;

  testbed::tests * mytestbed = new testbed::tests();
  testbed::set_mpi(mpi_info);
  testbed::set_filename("testing.log");
  mytestbed->setup_tests();

  mytestbed->add("sample");

  ADDABLE_FN_TYPE(second) myfun = ADDABLE_FN(second::setup, 1, "hi");

//  void (test_entity_second::*tmpfn)(int) = &test_entity_second::setup_member;
//Create tmpfn with correct signature
//  myfun = std::bind(tmpfn, std::placeholders::_1, 1);

  //RESOLVED_FN_TYPE(second, tmpfn)(int);
//  myfun = [&test_entity_second::setup_member](test_entity_second *){ return 
//  auto aBind = [&a](int i, int j){ return a(i, j); };
  mytestbed->add("second", myfun);
  mytestbed->add("second", (ADDABLE_FN_TYPE(second)) ADDABLE_FN(second::setup, 2, ""));
  //myfun = std::bind(&test_entity_second::setup_member, std::placeholders::_1, 2, 3);
//  myfun = ADDABLE_FN(2,3);
  myfun = ADDABLE_FN(second::setup, 5, "bum");
  mytestbed->add("second", myfun);

  mytestbed->print_available();

  mytestbed->run_tests();

  
  delete mytestbed;
  MPI_Finalize();

}
/** Adding tests to the remit
*
*For a test where all necessary setup can be done in the constructor, simply call mytestbed->add("name"); where name is the name the class is registered under. 
*For a simple setup function, with any number of parameters but no overloads, we bind together the setup function arguments to the values required, and pass that as second argument to add(). Using supplied macros: 
ADDABLE_FN_TYPE(second) myfun = ADDABLE_FN(name::functionname, 1, "hello"); etc
and then mytestbed->add("name", myfun)
Before the test is run, the setup function will be invoked with the arguments given. Multiple calls to add with different arguments may be made, each adding a new test object
For overloaded setup functions, there is an additional step. std::bind, which underlies the ADDABLE_FN macro, has to be told which overload is required. 
So we first do void (test_entity_second::*tmpfn)(int) = &test_entity_second::setup_member;
to create tmpfn with the correct signature, and then bind to the parameters of tmpfn like this myfun = std::bind(tmpfn, std::placeholders::_1, 1); Finally we call add("name", myfun) as before. 

*/

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
