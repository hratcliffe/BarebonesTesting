

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
std::cout<<std::setprecision(10);

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

static testbed::Registrar<test_entity_sample> registrar("sample");
//This string can be anything, but for sanity, make it the additional part of the test entity

int main(int argc, char ** argv){

  int ierr, rank, n_procs;

  ierr = MPI_Init(&argc, &argv);
  //Note any other command line arg processing should account for this...

  ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  ierr = MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
  
  mpi_info_struc mpi_info;
  mpi_info.rank = rank;
  mpi_info.n_procs = n_procs;
/*
  testbed::tests * mytestbed = new testbed::tests();
  testbed::set_mpi(mpi_info);
  testbed::set_filename("testing.log");
  mytestbed->setup_tests();
  test_entity_sample * mytest = new test_entity_sample();
  mytestbed->add_test(mytest);
  mytestbed->run_tests();
  
  delete mytestbed;
*/

  testbed::tests * mytestbed = new testbed::tests();
  testbed::set_mpi(mpi_info);
  testbed::set_filename("testing.log");
  mytestbed->setup_tests();
  mytestbed->add("sample");

  mytestbed->run_tests();
//  auto eg = testbed::test_factory::instance()->create("sample");
//  std::cout<<eg->name<<'\n';

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
