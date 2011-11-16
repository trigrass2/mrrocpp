/*
 * OutputBuffer.h
 *
 *  Created on: Nov 13, 2011
 *      Author: ptroja
 */

#ifndef OUTPUTBUFFER_H_
#define OUTPUTBUFFER_H_

#include <string>

#include "RemoteAgent.h"
#include "base/lib/xdr/xdr_oarchive.hpp"

/**
 * Remote data buffer proxy
 */
template <class T>
class OutputBuffer {
private:
	//! name of the buffer
	const std::string name;

	//! owner of the buffer
	RemoteAgent &owner;

public:
	//! Construct remote buffer proxy
	OutputBuffer(RemoteAgent & _owner, const std::string & _name)
		: name(_name), owner(_owner)
	{
	}

	//! Set the contents of the remote buffer
	void Send(const T & data) {
		xdr_oarchive<> oa;
		oa << name;
		oa << data;

		owner.Send(oa);
	}
};

#endif /* OUTPUTBUFFER_H_ */
