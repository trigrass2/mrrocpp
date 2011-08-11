/**
 * @file mp_t_stats.h
 * @brief Header file for stats task class.
 * @author Tomasz Bem (mebmot@wp.pl)
 * @ingroup stats
 * @date 13.05.2010
 */

#ifndef ECP_T_STATS_H_
#define ECP_T_STATS_H_

#include "base/ecp/ecp_task.h"
#include "generator/ecp/ecp_g_newsmooth.h"
#include "generator/ecp/ecp_g_constant_velocity.h"
#include "ecp_g_stats_generator.h"
#include "generator/ecp/ecp_g_sleep.h"

namespace mrrocpp {
namespace ecp {
namespace common {
namespace task {

/**
 * @brief Representation of Stats task in MRROC++.
 * @details Stats task is responsible for communication and proper information
 * flow on the MRROC++ side. It engages different generators to accomplish
 * the task.
 */
class Stats : public task
{

private:
	/**
	 * @brief Smooth trajectory generator.
	 * @detials Smooth generator used for moving a manipulator to proper
	 * position which is the first position from a trajectory read from
	 * VSP. The trajectory itself is also stored in VSP.
	 */
	common::generator::newsmooth* smoothGenerator;

	/**
	 * @brief Trajectory generator for position from stats VSP.
	 * @details The generator is started just after smooth generator
	 * reaches first position of a trajectory, and works until appropriate
	 * signal from VSP is sent START_BREAKING which indicate the moment
	 * when braking occurs and stops execution of generator.
	 */
	common::generator::stats_generator* statsGenerator;

public:
	Stats(lib::configurator &_config);
	~Stats();
	void mp_2_ecp_next_state_string_handler(void);
	void ecp_stop_accepted_handler();
};

} // namespace task
} // namespace irp6ot
} // namespace ecp
} // namespace mrrocpp

#endif /* ECP_T_STATS_H_ */
