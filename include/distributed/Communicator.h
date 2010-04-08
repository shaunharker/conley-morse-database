/*
 *  Communicator.h
 */


#ifndef _CMDP_COMMUNICATOR_
#define _CMDP_COMMUNICATOR_

#include "distributed/Message.h"
 
/** Communicator_Archetype.
    This code declares the interface for communicators.
*/

template < class Entity_Template >
class Communicator_Archetype {
 
public:
 
  typedef Entity_Template Entity;
  
  /* Memorized entities */
  Entity DIRECTOR;
  Entity SELF;
  
  /* Wildcards */
  Entity ANYSOURCE;
  int ANYTAG;
  
  /* Virtual deconstructor */
  virtual ~Communicator_Archetype ( void );
  
  /** initialize ( int argc, char * argv [] );
   *     - Responsible for initializing communications. 
   *     - Responsible for setting the values of DIRECTOR, SELF, ANYSOURCE, ANYTAG.
   * */
  virtual void initialize ( int argc, char * argv [] ) = 0; 
  
  /** finalize ( void ).
   *      - Responsible for finalizing communications. */ 
  virtual void finalize ( void ) = 0;

  /** send ( const Message & send_me, const Entity & target ).
   *    sends the contents of send_me to "target" 
   */
  virtual void send ( const Message & send_me, const Entity & target ) = 0;
 
  /** void receive ( Message * receive_me, Entity * sender, int tag, const Entity & source ).
   *   Receives a message from "source" with tag "tag" and stores it in "receive_me".
   *   Identity of sender is stored in "sender"
   *   If tag == ANYTAG, message is received with any tag. 
   *   If source == ANYSOURCE, message is received from any source.
   */
  virtual void receive ( Message * receive_me, Entity * sender, int tag, const Entity & source ) = 0; 
 
  /** bool probe ( int tag ); 
   *    Checks to see if there is an available message with tag "tag"
   */
  virtual bool probe ( int tag ) = 0; 
 
};

/** Simple_MPI_Communicator. 
     Follows the model of Communicator_Archetype */
	 
class MPI_Entity {
public:
  int name;
  bool operator < ( const MPI_Entity & right_hand_side ) const;
  bool operator == ( const MPI_Entity & right_hand_side ) const;
};
  
class Simple_MPI_Communicator : public Communicator_Archetype<MPI_Entity> {
private:
  int buffer_length;
  char * buffer;
public:
  ~Simple_MPI_Communicator ( void );
  void initialize ( int argc, char * argv [] ); 
  void finalize ( void );
  void send ( const Message & send_me, const Entity & target );
  void receive ( Message * receive_me, Entity * sender, int tag, const Entity & source ); 
  bool probe ( int tag ); 
};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "distributed/Communicator.hpp"
#endif

#endif
