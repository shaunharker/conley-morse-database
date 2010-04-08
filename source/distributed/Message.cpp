/*
 *  Message.cpp
 */

#include "distributed/Message.h"

Message::Message ( void ) {
  state = CLOSED;
}
 
Message::Message ( const Message & copy_me ) {
  state = CLOSED;
  str ( copy_me . str () ); // apparently stringbuf doesn't provide assignment
  tag = copy_me . tag;
}
 
Message::~Message ( void ) {
  close ();
}
 
std::string Message::str ( void ) const {
  close ();
  return data . str ();
}
 
void Message::str ( const std::string copy_contents ) {
  close ();
  data . str ( copy_contents );
}

void Message::open_for_reading ( void ) const {
  close ();
  input_archive = new boost::archive::binary_iarchive ( data );
  state = READING;
}
 
void Message::open_for_writing ( void ) {
  close ();
  output_archive = new boost::archive::binary_oarchive ( data );
  state = WRITING;
}
 
void Message::close ( void ) const {
  if ( state == READING ) delete input_archive;
  if ( state == WRITING ) delete output_archive;
  state = CLOSED;
}
