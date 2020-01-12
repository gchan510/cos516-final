#include <cstdlib>
#include "horn/EquivCheck.hpp"

using namespace ufo;
using namespace std;

int main (int argc, char ** argv)
{
  // add option here for unrolling
  if ( argc != 3 && argc != 5 ){
    outs() << "Two input files with CHCs should be given (with optional unrolling factors)\n";
    outs() << "Usage: equiv <file1> <file2> [unroll1 unroll2]\n";
    return 0;
  }

  if ( argc == 3 )
    equivCheck(string(argv[1]), string(argv[2]), 0, 0);
  else
    equivCheck(string(argv[1]), string(argv[2]), atoi(argv[3]), atoi(argv[4]));


  return 0;
}
