//
//  tests.cpp
//  
//
//  Created by Heather Ratcliffe on 10/11/2015.
//
//

#ifdef RUN_TESTS_AND_EXIT

#include <stdio.h>
#include <math.h>
#include <cmath>
#include "tests.h"

const mpi_info_struc mpi_info={1, 2};

const int err_codes[err_tot] ={TEST_PASSED, TEST_WRONG_RESULT, TEST_NULL_RESULT, TEST_ASSERT_FAIL, TEST_USERDEF_ERR1, TEST_USERDEF_ERR2, TEST_USERDEF_ERR3, TEST_USERDEF_ERR4};/**< List of error codes available*/

std::string err_names[err_tot]={"None", "Wrong result", "Invalid Null result", "Assignment or assertion failed", "", "", "", ""};/**< Names corresponding to error codes, which are reported in log files*/

tests::tests(){
  setup_tests();
}
tests::~tests(){
  cleanup_tests();
}

void tests::set_verbosity(int verb){
/** Set the verbosity of testing output, from 0 (minimal) to max_verbos.*/
  if((verb > 0)) this->verbosity = std::max(verb, max_verbos);

}

void tests::setup_tests(){
/** \brief Setup test bed
*
*Opens reporting file. Then instantiates all the test objects and adds them into the test_list
*/
  outfile = new std::fstream();
  outfile->open(filename.c_str(), std::ios::out);
  if(!outfile->is_open()){
    my_print("Error opening "+filename, mpi_info.rank);
    //can't log so return with empty test list
    return;
  }

  test_entity * test_obj;

/*  test_obj = new test_entity_reader();
  add_test(test_obj);*/
  //these two lines are needed for each test you want to do.

}

void tests::add_test(test_entity * test){
  /** Adds a test to the list to be performed*/
  test_list.push_back(test);
}

void tests::report_err(int err, int test_id){
/** \brief Log error
*
* Logs error text corresponding to code err for test defined by test_id. Errors are always recorded.*/
  if(test_id == -1) test_id = current_test_id;
  if(err ==TEST_PASSED) set_colour('b');
  else set_colour('r');
  my_print(outfile, get_printable_error(err, test_id), mpi_info.rank);
  my_print(nullptr, get_printable_error(err, test_id), mpi_info.rank);
  set_colour();

}

void tests::report_info(std::string info, int verb_to_print, int test_id){
/** \brief Other test info
*
*Records string info to the tests.log file and to screen, according to requested verbosity.
*/
  if(test_id == -1) test_id = current_test_id;
  if(verb_to_print <= this->verbosity){
    my_print(outfile, info, mpi_info.rank);
    my_print(nullptr, info, mpi_info.rank);
  
  }

}

std::string tests::get_printable_error(int err, int test_id){
/** \brief Make an error message
*
* Converts error code to printable string, adds code for reference and adds test name. Note code is bitmask and additional errors are appended together
*/
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

void tests::set_colour(char col){
/** \brief Set output text colour
*
*Set terminal output colour using std escape sequences. Accepts no argument to return to default, or rgb, cmyk and white to test text colour. NB technically not MPI safe. Use sparingly to highlight important information.
*/

  testbed::myprint(this->get_color_escape(col));
}

std::string tests::get_color_escape(char col){
/** \brief Get color string
*
* Returns the terminal escape string to set given colour
*/
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

void tests::cleanup_tests(){
/** \brief Delete test objects
*
*
*/
  if(outfile->is_open()){
    this->report_info("Testing complete and logged in " +filename, 0);
    outfile->close();
  }else{
    this->report_info("No logfile generated", 0);

  }
  delete outfile;
  for(current_test_id=0; current_test_id< (int)test_list.size(); current_test_id++){
    delete test_list[current_test_id];
  
  }

  
}

void tests::run_tests(){
/** \brief Run scheduled tests
*
*Runs each test in list and reports total errors found
*/

  int total_errs = 0;
  for(current_test_id=0; current_test_id< (int)test_list.size(); current_test_id++){
    total_errs += (bool) test_list[current_test_id]->run();
    //Add one if is any error returned
  }
  this->set_colour('r');
  this->report_info(mk_str(total_errs)+" failed tests", mpi_info.rank);
  this->set_colour();

}

//#ifdef USE_MPI
namespace testbed{
void my_print(std::string text, int rank, int rank_to_write){
/** \brief Write output
*
* Currently dump to term. Perhaps also to log file. Accomodates MPI also. Set rank_to_write to -1 to dump from all. Default value is 0
*/
  if(rank == rank_to_write || rank_to_write == -1){
    std::cout<< text<<std::endl;
  }

}
void my_print(std::fstream * handle, std::string text, int rank, int rank_to_write){
/** \brief Write output
*
* Currently dump to term. Perhaps also to log file. Accomodates MPI also. Set rank_to_write to -1 to dump from all. Default value is 0
*/
  if((rank == rank_to_write || rank_to_write == -1) && handle!=nullptr){
    *handle<<text<<std::endl;
  }else if(rank == rank_to_write || rank_to_write == -1){
    std::cout<<text<<std::endl;

  }

}
//#else

void my_print(std::string text){
    std::cout<< text<<std::endl;
}

void my_print(std::fstream * handle, std::string text){
  if(handle!=nullptr){
    *handle<<text<<std::endl;
  }else{
    std::cout<<text<<std::endl;
  }

}

std::string mk_str(int i){

  char buffer[25];
  std::sprintf(buffer, "%i", i);
  std::string ret = buffer;
  return ret;
  
}

std::string mk_str(double i, bool noexp){

  char buffer[25];
  if(noexp) std::snprintf(buffer, 25, "%f", i);
  else std::snprintf(buffer, 25, "%e", i);
  std::string ret = buffer;
  return ret;
  
}

std::string mk_str(bool b){

  if(b) return "1";
  else return "0";

}

std::string mk_str(long double i, bool noexp){return mk_str((double) i, noexp);};
std::string mk_str(float i, bool noexp){return mk_str((double) i, noexp);};
}

//#endif

#endif