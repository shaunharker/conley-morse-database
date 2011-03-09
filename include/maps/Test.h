/* Test Map */

#ifndef CMDP_TESTMAP_H
#define CMDP_TESTMAP_H

#include "toplexes/Adaptive_Cubical_Toplex.h"
#include <cmath>


class Point {
public:
  double* data_;

	Point(){ data_ = NULL;}
	Point( unsigned int dimension ){
		data_ = new double[ dimension ];
		for( unsigned int i = 0; i < dimension; ++i)
			data_[i] = 0;
	}
  
	~Point(){ if ( data_ != NULL ) delete[] data_;}

  const double & operator () ( const unsigned int pos ) const { return data_[pos];}

  double & operator () ( const unsigned int pos ) { return data_[pos];}
	

	double Get(const  unsigned  int pos ) const { return data_[ pos ];}
	void Set( const unsigned int pos, double value ){ data_[ pos ] = value;}
	/* This will have to change when Geometric_Description changes */
	void Mid_Point( const Adaptive_Cubical::Geometric_Description & rectangle ){
		if ( data_ != NULL ) delete[] data_;
		data_ = new double[ rectangle.lower_bounds.size() ];
		for ( unsigned int dim = 0; dim < rectangle.lower_bounds.size(); ++dim ){
			data_[ dim ] = ( rectangle.lower_bounds[ dim ] + rectangle.upper_bounds [ dim ] ) / 2;
		}
	}

};

struct SquareMatrix{
private:
	unsigned int dimension_;
	double* data_;
public:
	SquareMatrix(){ data_ = NULL; }
	SquareMatrix( unsigned int dim ){ dimension_ = dim; data_ = new double[ dim * dim ]; }
	~SquareMatrix(){ if( data_ != NULL ) delete[] data_; }
  
  const double & operator () ( const unsigned int i, const unsigned int j  ) const { return data_[ i * dimension_ + j ];}

	double & operator () ( const unsigned int i, const unsigned int j ) { return data_[ i * dimension_ + j ];}
	
	double Get(const unsigned int i, const unsigned int j) const { return data_[ i * dimension_ + j ];}
	void Set(const unsigned int i, const unsigned int j, double value ){ data_[ i * dimension_ + j ] = value; }
/*
	Point operator * ( const Point & rhs ) const {
		Point result( dimension );
		for( unsigned int i = 0; i < dimension; ++i ) {
			for( unsigned int j = 0; j < dimension; ++j ) {
				result.data_[ i ] +=  Get( i, j ) * rhs.data_[ j ];
			}
		}
	}
*/
	void AbsolutValue( Point & abs_value ){
		//std::cout << "Entering abs value.\n";
		for( unsigned int i = 0; i < dimension_; ++i ) {
			for( unsigned int j = 0; j < dimension_; ++j ) {
				//std::cout << "i = " << i << " j = " << j <<"\n";
				abs_value.data_[ i ] +=   (double)fabs( Get(i,j)  );
				
				//std::cout << "Val  = " <<  data_[ i * dimension_ + j ] << "\n";
				//std::cout << "Abs = " << abs_value.data_[ i ] << "\n";
			}
		}
		//std::cout << "Leaving absolute value. \n";
	}
};

#define LAMBDA 3.3f


/* This structure evaluates
   value, derrivative and error of the map
*/
struct ComputationalMap{

public:
	Adaptive_Cubical::Geometric_Description parameters;

	void Test( double x){};

	/* Constructor loads the parameters */
	ComputationalMap(){}
	ComputationalMap( const Adaptive_Cubical::Geometric_Description & rectangle ) : parameters ( rectangle ) { }

	void F( const Point & x, Point  &fx) const {
		fx( 0 ) = LAMBDA * x( 0 ) * ( 1.0f - x( 0 ) );
		fx( 1 ) = LAMBDA * x( 1 ) * ( 1.0f - x( 1 ) );
	}
	void DF( const Point & x, SquareMatrix & DFx ) const {
		DFx ( 0, 0 ) = LAMBDA * ( -2.0f * x( 0 ) + 1.0f );
		DFx ( 0, 1 ) = 0;
		DFx ( 1, 0 ) = 0;
		DFx ( 1, 1 ) = LAMBDA * ( -2.0f * x( 1 ) + 1.0f );
	}
	void Error( const Point & x, Point & error_value ) const {
		error_value ( 0 ) = 0.01;
		error_value ( 1 ) = 0.01;
	}



};


struct TestMap {

ComputationalMap MyMap;

public:
	/* Is this the way we want to pass the parameters ? Probbly we want just cubes and we will mess with this*/
	TestMap ( const Adaptive_Cubical::Geometric_Description & rectangle ) /*: MyMap ( rectangle )*/ { }



	Adaptive_Cubical::Geometric_Description operator () (
    const Adaptive_Cubical::Geometric_Description & rectangle ) const {

		
		unsigned int dimension = rectangle.lower_bounds.size();
		Adaptive_Cubical::Geometric_Description return_value ( dimension );

		/* I suppose that the box is a square */
		double box_width = ( rectangle.upper_bounds[ 0 ] - rectangle.lower_bounds [ 0 ] ) / 2;

		Point box_mid_point;
		Point fx(dimension);
		Point error(dimension);
		SquareMatrix DFx(dimension);
		Point baunding_box(dimension);

		box_mid_point.Mid_Point( rectangle );
		MyMap.F( box_mid_point, fx );
		MyMap.DF( box_mid_point, DFx );
		MyMap.Error( box_mid_point, error );

		DFx.AbsolutValue( baunding_box );
		//std::cout << "Dimension" << dimension << "\n";
		for( unsigned int dimension_index = 0; dimension_index < dimension; ++dimension_index ){
			double fx1 = fx.Get( dimension_index );
			//std::cout << "fx = " << fx1 << "\n";
			return_value.lower_bounds[ dimension_index ] = fx.Get( dimension_index ) - baunding_box.Get( dimension_index ) * box_width - error.Get( dimension_index );
			return_value.upper_bounds[ dimension_index ] = fx.Get( dimension_index ) + baunding_box.Get( dimension_index ) * box_width + error.Get( dimension_index );
			double bb = baunding_box.Get( dimension_index );
			/*
      std::cout << "Bounding Box = " << bb << "\n";
			std::cout << " O Lower bound " <<  rectangle.lower_bounds[ dimension_index ] << " Upper bound " << rectangle.upper_bounds[ dimension_index ] << "\n";
			std::cout << "Lower bound " <<  return_value.lower_bounds[ dimension_index ] << " Upper bound " << return_value.upper_bounds[ dimension_index ] << "\n";
       */
		}

		//return_value.lower_bounds[ 0 ] = ( rectangle.lower_bounds [ 0 ] + rectangle.upper_bounds [ 0 ] ) / 2.0f - ( - rectangle.lower_bounds [ 0 ] + rectangle.upper_bounds [ 0 ] ) / 5.0f;
		//return_value.upper_bounds[ 0 ] = ( rectangle.lower_bounds [ 0 ] + rectangle.upper_bounds [ 0 ] ) / 2.0f + ( - rectangle.lower_bounds [ 0 ] + rectangle.upper_bounds [ 0 ] ) / 5.0f;
		//return_value.lower_bounds[ 1 ] =( rectangle.lower_bounds [ 0 ] + rectangle.upper_bounds [ 0 ] ) / 2.0f - ( - rectangle.lower_bounds [ 1 ] + rectangle.upper_bounds [ 1 ] ) / 5.0f;
		//return_value.upper_bounds[ 1 ] = ( rectangle.lower_bounds [ 0 ] + rectangle.upper_bounds [ 0 ] ) / 2.0f  + ( - rectangle.lower_bounds [ 1 ] + rectangle.upper_bounds [ 1 ] ) / 5.0f;

		return return_value;
	}

};

#endif
