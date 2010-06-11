#include <boost/thread/xtime.hpp>
#include <iostream>

#include "edp/common/edp.h"
#include "edp/common/edp_e_manip.h"
#include "lib/mis_fun.h"
#include "edp/common/reader.h"
#include "kinematics/common/kinematic_model_with_tool.h"

namespace mrrocpp {
namespace edp {
namespace sensor {

//!< watek do komunikacji ze sprzetem
void force::operator()(void)
{
	// Set thread parameters
	lib::set_thread_priority(pthread_self(), MAX_PRIORITY - 1);
	lib::set_thread_name("force sensor");

	// Do not touch the hardware when running in test mode
	if(!test_mode) {
		connect_to_hardware();
	}

	// Notify the caller, that the thread is ready
	thread_started.command();

	try {
		configure_sensor();
	}
	// TODO: transport exception to main thread

	catch (...) {
		std::cerr << "unidentified error force thread w EDP" << std::endl;
	}

	while (!TERMINATE) //!< for (;;)
	{
		try {
			if (new_edp_command) {
				boost::mutex::scoped_lock lock(mtx);
				// TODO: this should be handled with boost::bind functor parameter
				switch (command)
				{
					case (common::FORCE_SET_TOOL):
						set_force_tool();
						break;
					case (common::FORCE_CONFIGURE):
						configure_sensor();
						break;
					default:
						break;
				}
				set_command_execution_finish();
			} else {
				if(!test_mode) {
					wait_for_event();

					initiate_reading();

					get_reading();
				} else {
					initiate_reading();

					get_reading();
				}
			}
		}
		// TODO: transport exception to main thread
		catch (...) {
			std::cerr << "unidentified error in EDP force thread" << std::endl;
		}
	}
}

force::force(common::manip_effector &_master) :
			is_reading_ready(false), //!< nie ma zadnego gotowego odczytu
			gravity_transformation(NULL), master(_master), test_mode(true), TERMINATE(false),
			is_sensor_configured(false), //!< czujnik niezainicjowany
			new_edp_command(false)
{
	//! Lokalizacja procesu wywietlania komunikatow SR
	sr_msg
			= new lib::sr_vsp(lib::EDP,
					master.config.return_attach_point_name(lib::configurator::CONFIG_SERVER, "edp_vsp_attach_point"),
					master.config.return_attach_point_name(lib::configurator::CONFIG_SERVER, "sr_attach_point", UI_SECTION),
					true);

	if (master.config.exists(FORCE_SENSOR_TEST_MODE)) {
		test_mode = master.config.value <int> (FORCE_SENSOR_TEST_MODE);
	}

	if (test_mode) {
		sr_msg->message("Force sensor test mode activated");
	}
}

force::~force()
{
	if (gravity_transformation)
		delete gravity_transformation;
	delete sr_msg;
}

void force::set_force_tool(void)
{
	lib::K_vector gravity_arm_in_sensor(next_force_tool_position);
	lib::Homog_matrix frame = master.return_current_frame(common::WITH_TRANSLATION);
	gravity_transformation->defineTool(frame, next_force_tool_weight, gravity_arm_in_sensor);

	current_force_tool_position = next_force_tool_position;
	current_force_tool_weight = next_force_tool_weight;
}

void force::set_command_execution_finish() // podniesienie semafora
{
	new_edp_command = false;
	new_command_synchroniser.command();
}

void force::get_reading()
{
	lib::Ft_vector zero_ft;

	master.force_msr_upload(zero_ft);

	is_reading_ready = true;

	// scope-locked reader data update
	{
		boost::mutex::scoped_lock lock(master.rb_obj->reader_mutex);

		master.rb_obj->step_data.force = zero_ft;
	}
}

void force::configure_sensor()
{
	// polozenie kisci bez narzedzia wzgledem bazy
	lib::Homog_matrix frame = master.return_current_frame(common::WITH_TRANSLATION); // FORCE Transformation by Slawomir Bazant

	// jesli nie powolano jeszcze obiektu
	if (!gravity_transformation) {
		lib::Homog_matrix sensor_frame;
		if (master.config.exists("sensor_in_wrist")) {
			lib::Xyz_Angle_Axis_vector tab;

			// TODO: this should be done with boost::tokenizer
			char *tmp = strdup(master.config.value <std::string> ("sensor_in_wrist").c_str());
			char *toDel = tmp;

			for (int i = 0; i < 6; i++)
				tab[i] = strtod(tmp, &tmp);

			free(toDel);

			sensor_frame = lib::Homog_matrix(tab);
		} else {
			sensor_frame = lib::Homog_matrix(0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1, 0.09);
		}

		double weight = master.config.value <double> ("weight");

		double point[3];
		char *tmp = strdup(master.config.value <std::string> ("default_mass_center_in_wrist").c_str());
		char* toDel = tmp;
		for (int i = 0; i < 3; i++)
			point[i] = strtod(tmp, &tmp);
		free(toDel);

		lib::K_vector pointofgravity(point);
		gravity_transformation = new lib::ForceTrans(frame, sensor_frame, weight, pointofgravity);
	} else {
		gravity_transformation->synchro(frame);
	}

	is_sensor_configured = true;

	sr_msg->message("EDP Sensor configured");
}

void force::initiate_reading(void)
{
	if (!is_sensor_configured) {
		BOOST_THROW_EXCEPTION(
				lib::exception::Fatal_error() <<
				lib::exception::error_code(SENSOR_NOT_CONFIGURED)
		);
	}
}

void force::set_current_ft_reading(const lib::Ft_vector& current_ft)
{
	lib::Homog_matrix frame = master.return_current_frame(common::WITH_TRANSLATION);

	lib::Ft_vector output = gravity_transformation->getForce(current_ft, frame);

	master.force_msr_upload(output);

	// scope-locked reader data update
	{
		boost::mutex::scoped_lock lock(master.rb_obj->reader_mutex);

		master.rb_obj->step_data.force = current_ft;
	}

	is_reading_ready = true;
}

void force::wait_for_event()
{
	// Sleep for a short period of time
	boost::this_thread::sleep(boost::posix_time::millisec(2));
}

} // namespace sensor
} // namespace edp
} // namespace mrrocpp
