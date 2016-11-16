/*
 * dpdk_link.h
 *
 *  Created on: Nov 16, 2016
 *      Author: ved
 */

#ifndef DPDK_LINK_H_
#define DPDK_LINK_H_


/*
 * TODO: VED
 * This file should contain
 * Step 1:
 * 1) Dpdk alternatives for read() and write()
 * 2) Should convert the received data from mbuf to a new copy of Buffer
 *
 * Step 2:
 * 1) Should take care of links using only a single copy of Buffer/mbuf
 *
 * Later Link should be a base class with two implementations non_dpdk and dpdk
 * as two individual derived classes
 * */

class DpdkLink{
public:
	void init();
	void read();
	void write();
};


#endif /* DPDK_LINK_H_ */
