/*
 *  Conley_Index_Job.hpp
 */

#include "chomp/ConleyIndex.h"

#include "database/program/Configuration.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "database/structures/MorseGraph.h"
#include "database/structures/Database.h"
#include "database/structures/Grid.h"
#include "database/structures/PointerGrid.h"

#include "chomp/Rect.h"
#include <vector>
#include <string>


#include "chomp/Matrix.h"
#include "chomp/PolyRing.h"

#include <boost/thread.hpp>
#include <boost/chrono/chrono_io.hpp>

typedef chomp::SparseMatrix < chomp::PolyRing < chomp::Ring > > PolyMatrix;

class SNFThread {

private:

  PolyMatrix * U;
  PolyMatrix * Uinv;
  PolyMatrix * V;
  PolyMatrix * Vinv;
  PolyMatrix * D;
  const PolyMatrix & A;
public:
  bool * result;
  SNFThread( PolyMatrix * U,
       PolyMatrix * Uinv,
       PolyMatrix * V,
       PolyMatrix * Vinv,
       PolyMatrix * D,
       const PolyMatrix & A, 
       bool * result ) 
  : U(U), Uinv(Uinv), V(V), Vinv(Vinv), D(D), A(A), result(result) {}

  void operator () ( void ) {
    try {
      SmithNormalForm ( U, Uinv, V, Vinv, D, A );
      *result = true;
    } catch ( ... /* boost::thread_interrupted& */) {
      *result = false;
    }
  }
};

std::vector<std::string> conley_index_string ( const chomp::ConleyIndex_t & ci, int time_out = 180 ) {
  using namespace chomp;
  std::cout << "conley index string\n";
  std::vector<std::string> result;
  if ( ci . undefined () ) { 
    std::cout << "undefined.\n";
    return result;
  }
  for ( unsigned int i = 0; i < ci . data () . size (); ++ i ) {
    std::cout << "dimension is " << i << "\n";
    std::stringstream ss;
    PolyMatrix poly = ci . data () [ i ];
    
    int N = poly . number_of_rows ();
    PolyRing<Ring> X;
    X . resize ( 2 );
    X [ 1 ] = Ring ( -1 );
    for ( int i = 0; i < N; ++ i ) {
      poly . add ( i, i, X );
    }
    PolyMatrix U, Uinv, V, Vinv, D;

    // use a thread to perform the following line:    
    //      SmithNormalForm ( &U, &Uinv, &V, &Vinv, &D, poly );
    bool computed;
    SNFThread snf ( &U, &Uinv, &V, &Vinv, &D, poly, &computed );
    boost::thread t(snf);
    if ( not t . try_join_for ( boost::chrono::seconds( time_out ) ) ) {
      t.interrupt();
      t.join();
    }
    if ( not computed ) {
      result . push_back ( std::string ( "Problem computing SNF.\n") );
      continue;
    }
    // end threading

    bool is_trivial = true;
    PolyRing < Ring > x;
    x . resize ( 2 );
    x [ 1 ] = Ring ( 1 );
    for ( int j = 0; j < D . number_of_rows (); ++ j ) {
      PolyRing < Ring > entry = D . read ( j, j );
      while ( ( entry . degree () >= 0 )
             && ( entry [ 0 ] == Ring ( 0 ) )) {
        entry = entry / x;
      }
      if ( entry . degree () <= 0 ) continue;
      is_trivial = false;
      ss << "   " << entry << "\n";
    }
    if ( is_trivial ) ss << "Trivial.\n";
    result . push_back ( ss . str () );
    std::cout << "Wrote the poly " << ss . str () << "\n";
  }
  return result;
}

template <class PhaseGrid >
void Conley_Index_Job ( Message * result , const Message & job ) {
  using namespace chomp;
  // Read job
  size_t job_number;
  uint64_t incc;
  chomp::Rect geo;
  uint64_t ms;
  int PHASE_SUBDIV_INIT;
  int PHASE_SUBDIV_MIN;
  int PHASE_SUBDIV_MAX;
  int PHASE_SUBDIV_LIMIT;
  Rect PHASE_BOUNDS;
  std::vector < bool > PERIODIC;
  job >> job_number;
  job >> incc;
  job >> geo;
  job >> ms;
  job >> PHASE_SUBDIV_INIT;
  job >> PHASE_SUBDIV_MIN;
  job >> PHASE_SUBDIV_MAX;
  job >> PHASE_SUBDIV_LIMIT;
  job >> PHASE_BOUNDS;
  job >> PERIODIC;
  
  std::cout << "CIJ: job_number = " << job_number << "  (" << geo << ", " <<  ms << ")\n";

  // Compute Morse Graph
  MorseGraph mg;
  
  boost::shared_ptr<Grid> phase_space ( new PhaseGrid );
  phase_space -> initialize ( PHASE_BOUNDS, PERIODIC );
  
     std::cout << "CIJ: geo = " << geo << "\n";
  
  GeometricMap map ( geo );

  std::cout << "CIJ: calling Compute_Morse_Graph\n";
  
  Compute_Morse_Graph ( &mg,
                        phase_space,
                        map,
                        PHASE_SUBDIV_INIT,
                        PHASE_SUBDIV_MIN,
                        PHASE_SUBDIV_MAX,
                        PHASE_SUBDIV_LIMIT );
  
    std::cout << "CIJ: returned from Compute_Morse_Graph\n";

  // Select Subset
    std::cout << "periodic = " << (PERIODIC[0] ? "yes" : "no" ) << "\n";
    std::cout << "phase bounds = " << PHASE_BOUNDS << "\n";
    std::cout << "incc = " << incc << "\n";
    std::cout << "ms = " << ms << "\n";
    std::cout << "num vertices = " << mg . NumVertices () << "\n";
    std::cout << "CIJ: size of phase space = " << phase_space -> size () << "\n";
    std::cout << "CIJ: size of morse set = " << mg . grid ( ms ) -> size () << "\n";
    std::cout << "phase space grid type: " << typeid( * phase_space ).name() << "\n";
    std::cout << "ms grid type: " << typeid( * mg . grid ( ms ) ).name() << "\n";
  typedef std::vector < Grid::GridElement > Subset;
  Subset subset = phase_space -> subset ( * mg . grid ( ms ) );

  std::cout << "CIJ: calling Conley_Index on Morse Set " << ms << "\n";
  
  // Compute Conley Index Record of Morse Set
  ConleyIndex_t ci_matrix;
  ConleyIndex ( & ci_matrix, // ConleyIndex_t
                *phase_space,
                subset,
                map );

  std::cout << "CIJ: producing Conley Index polynomial strings \n";

  CI_Data ci_data;
  ci_data . conley_index = conley_index_string ( ci_matrix );
 
  
  // Return Result
  * result << job_number;
  * result << incc;
  * result << ci_data;
  
}
