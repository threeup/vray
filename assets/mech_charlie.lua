-- Short & Wide mech variant (Charlie)
-- Stocky frame, wide stance, heavily armored.

local scale = 0.46

mech = {
    scale = scale,
    stance_width = 0.70,

    foot_width = 0.48,
    foot_length = 0.95,
    foot_height = 0.32,
    foot_bottom_back_frac = 0.50,
    foot_bottom_front_frac = 0.78,
    foot_top_back_frac = 0.36,
    foot_top_front_frac = 0.18,
    foot_top_width_scale = 0.55,
    foot_y_offset_frac = 0.32,
    foot_z_offset_frac = 0.11,

    ankle_radius = 0.18,
    lower_leg_bottom = 0.18,
    lower_leg_top = 0.28,
    lower_leg_height = 0.95,
    knee_radius = 0.32,
    knee_z_offset = 0.11,
    upper_leg_bottom = 0.28,
    upper_leg_top = 0.38,
    upper_leg_height = 1.05,
    thigh_angle_deg = -18.0,
    upper_leg_extra_y = 0.04,
    hip_radius = 0.18,
    hip_length = 0.36,
    hip_x_offset = 0.66,

    pelvis_w = 1.40,
    pelvis_h = 0.48,
    pelvis_d = 0.90,
    pelvis_y = 2.75,

    shoulder_sphere_r = 0.34,
    shoulder_y = 4.50,
    shoulder_x = 0.92,
    shoulder_sphere_inset = 0.05,

    shield_angle_deg = -26.0,
    shield_rot_y_deg = 9.0,
    shield_dx = 0.09,
    shield_dy = 0.28,
    shield_w = 0.52,
    shield_h = 0.44,
    shield_t = 0.09,
    shoulder_cube = 0.34,
    armature_w = 0.26,
    armature_h = 0.18,
    armature_d = 0.18,
    armature_offset = 0.18,

    plasma_radius = 0.10,
    plasma_length = 1.65,
    plasma_y = -0.22,
    plasma_z = 0.36,
    plasma_x = 0.28,
    plasma_x2 = 0.14,
    plasma_shroud_len_frac = 0.34,
    plasma_shroud_radius_scale = 1.7,
    plasma_shroud_taper = 0.9,
    plasma_shroud_offset_frac = -0.32,
    plasma_barrel_len_frac = 0.62,
    plasma_barrel_offset_frac = 0.14,
    plasma_muzzle_len_frac = 0.10,
    plasma_muzzle_radius_scale = 1.15,
    plasma_muzzle_tip_scale = 1.08,
    plasma_muzzle_offset_frac = 0.46,

    rocket_w = 0.62,
    rocket_h = 0.70,
    rocket_d = 0.52,
    rocket_z = 0.18,
    rocket_x = 0.28,

    torso_r = 0.65,
    torso_h = 1.90,
    torso_y = 3.90,

    neck_r = 0.18,
    neck_h = 0.32,
    neck_y = 5.15,

    head_w = 0.42,
    head_h = 0.48,
    head_z = 0.11,

    left_weapon = 1,  -- rocket
    right_weapon = 1, -- rocket
}
