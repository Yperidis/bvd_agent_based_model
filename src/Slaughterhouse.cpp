#include "Slaughterhouse.h"
#include "Cow.h"
#include "Events.h"
#include "Herd.h"
#include "System.h"
#include "SlaughterHouseManager.h"
#include <iostream>
Slaughterhouse::Slaughterhouse( System* s ) :Farm(s)
{
  my_herds.push_back(new Herd( this ) );
  this->manager = new SlaughterHouseManager(this, s);
  this->myType = SLAUGHTERHOUSE;

}

Slaughterhouse::~Slaughterhouse()
{
  for (auto h : my_herds )
    delete h;
}

void Slaughterhouse::push_cow(Cow *c )
{
  //The cows enter the slaughterhouse...
  my_herds.front()->push_cow( c );
  system->schedule_event( new Event( system->current_time() , Event_Type::SLAUGHTER , c->id() ) );
  //and they never come back...
}

void Slaughterhouse::pull_cow( Cow *c )
{
  //Only meat here.
}
