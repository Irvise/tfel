/*!
 * \file   src/System/rfstream.cxx
 * \brief  
 * 
 * \author Helfer Thomas
 * \date   14 nov 2007
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#include<iostream>

#include"TFEL/System/System.hxx"
#include"TFEL/System/rfstream.hxx"

namespace tfel
{

  namespace system
  {
    
    rfstream::rfstream()
      : basic_rstream<rfstream,stream_traits<rfstream> >(),
	std::shared_ptr<int>(new int(-1))
    {} // end of rfstream::rfstream

    rfstream::rfstream(const rfstream& src)
      : basic_rstream<rfstream,stream_traits<rfstream> >(src),
	std::shared_ptr<int>(src)
    {} // end of rfstream::rfstream

    rfstream &
    rfstream::operator=(const rfstream& src)
    {
      if(this==&src){
	return *this;
      }
      this->close();
      basic_rstream<rfstream,stream_traits<rfstream> >::operator=(src);
      std::shared_ptr<int>::operator=(src);
      return *this;
    } // end of rfstream::operator

    rfstream::rfstream(const std::string& name, const int flags)
      : basic_rstream<rfstream,stream_traits<rfstream> >(),
	std::shared_ptr<int>(new int(-1))
    {
      this->open(name,flags);
    } // end of rfstream::rfstream

    void
    rfstream::open(const std::string& name,
		   const int flags)
    {
      using namespace std;
      int fd;
      if(*(this->get())!=-1){
	// closing the previous file
	this->close();
      }
      fd=::open(name.c_str(),flags);
      if(fd==-1){
	string msg("rfstream::open : ");
	msg += "failed to open file "+name+".";
	systemCall::throwSystemError(msg,errno);
      }
      shared_ptr<int>::operator=(shared_ptr<int>(new int(fd)));
    } // end of rfstream::open

    void
    rfstream::close(void)
    {
      using namespace std;
      if(*(this->get())==-1){
	return;
      }
      if(this->unique()){
	if(::close(*(this->get()))==-1){
	  string msg("rfstream::close : ");
	  msg += "failed to close file.";
	  systemCall::throwSystemError(msg,errno);
	}
      }
      shared_ptr<int>::operator=(shared_ptr<int>(new int(-1)));
    } // end of rfstream::close

    int
    rfstream::getFileDescriptor(void) const
    {
      return *(this->get());
    } // end of rfstream::getFileDescriptor

    rfstream::~rfstream()
    {
      this->close();
    } // end of rfstream::~rfstream
    
  } // end of namespace System  
  
} // end of namespace tfel  
