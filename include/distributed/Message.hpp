/*
 * Message.hpp
 */
 
template < class T >
Message & Message::operator << ( const T & insert_me ) {
  if ( state != WRITING ) open_for_writing ();
  *output_archive << insert_me;
  return *this;
}
 
template < class T >
const Message & Message::operator >> ( T & extract_me ) const {
  if ( state != READING ) open_for_reading ();
  *input_archive >> extract_me;
  return *this;
}
