/* Database */


template < class Geometric >
GridElementRecord::GridElementRecord ( const Geometric & g ) :
lower_bounds_(g . lower_bounds), upper_bounds_ ( g . upper_bounds ) {}

template < class Geometric, class CMG >
ParameterBoxRecord::ParameterBoxRecord ( int id_, const Geometric & g, const CMG & cmg) :
id_ ( id_ ), ge_ ( g ) {
  num_morse_sets_ = cmg . NumVertices ();
  for ( typename CMG::EdgeIteratorPair startstop = cmg . Edges (); 
        startstop . first != startstop . second; ++ startstop . first ) {
    partial_order_ . push_back ( std::make_pair ( startstop . first -> first,
                                                 startstop . first -> second ) );
  }
}
