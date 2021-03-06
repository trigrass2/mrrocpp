
add_subdirectory (weight_measure)
add_subdirectory (bias_edp_force)
add_subdirectory (force_tool_change)
add_subdirectory (tff_gripper_approach)
add_subdirectory (tff_nose_run)
add_subdirectory (tff_rubik_face_rotate)
add_subdirectory (transparent)
add_subdirectory (sleep)

add_library(ecp_mp_generators
ecp_mp_g_common.cc
)


target_link_libraries(ecp_mp_generators 
ecp_mp_g_tff_nose_run
ecp_mp_g_tff_gripper_approach
)

add_library(ecp_generators
	ecp_taught_in_pose.cc
	ecp_g_teach_in.cc
	ecp_g_delta.cc
	ecp_g_operator_reaction_condition.cc
	ecp_g_visual_servo_manager.cc
	ecp_g_visual_servo.cc
	ecp_g_get_position.cc
	ecp_g_constant_velocity.cc
	ecp_g_newsmooth.cc
        ecp_g_spline.cc
	velocity_profile_calculator/bang_bang_profile.cc
	velocity_profile_calculator/constant_velocity_profile.cc
        velocity_profile_calculator/spline_profile.cc
	trajectory_interpolator/constant_velocity_interpolator.cc
	trajectory_interpolator/bang_bang_interpolator.cc
        trajectory_interpolator/spline_interpolator.cc
)

target_link_libraries(ecp_generators 
ecp_g_weight_measure
ecp_g_bias_edp_force 
ecp_g_force_tool_change
ecp_g_tff_gripper_approach
ecp_g_tff_nose_run
ecp_g_tff_rubik_face_rotate
ecp_g_transparent
ecp_g_sleep
)



install(TARGETS ecp_generators ecp_mp_generators DESTINATION lib)
