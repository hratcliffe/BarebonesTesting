

#include <stdio.h>
#include "tests.h"
//#include "test_min.h"

int main(int argc, char ** argv){


mpi_info_struc aa={10, 0};
mpi_info_struc bb={0,0};

std::cout<<(aa==mpi_info_null)<<'\n';
std::cout<<(bb==mpi_info_null)<<'\n';

//tests * test_bed=new tests();

int i=100;
double d=123e3;
std::cout<<testbed::mk_str(i)<<" "<<testbed::mk_str(d)<<" "<<testbed::mk_str(d, true)<<'\n';

//" "<<mk_str(d, true)






}
