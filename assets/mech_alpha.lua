-- Tall & Skinny mech variant (Alpha)
-- Long legs, narrow body, nimble frame.

local scale = 0.35

mech = {
    scale = scale,
    stance_width = 0.50,

    foot_width = 0.32,
    foot_length = 0.85,
    foot_height = 0.28,
    foot_bottom_back_frac = 0.48,
    foot_bottom_front_frac = 0.85,
    foot_top_back_frac = 0.42,
    foot_top_front_frac = 0.22,
    foot_top_width_scale = 0.70,
    foot_y_offset_frac = 0.34,
    foot_z_offset_frac = 0.12,

    ankle_radius = 0.12,
    lower_leg_bottom = 0.12,
    lower_leg_top = 0.18,
    lower_leg_height = 1.60,
    knee_radius = 0.20,
    knee_z_offset = 0.08,
    upper_leg_bottom = 0.18,
    upper_leg_top = 0.26,
    upper_leg_height = 1.75,
    thigh_angle_deg = -12.0,
    upper_leg_extra_y = 0.07,
    hip_radius = 0.24,
    hip_length = 0.45,
    hip_x_offset = 0.78,

    pelvis_w = 0.95,
    pelvis_h = 0.36,
    pelvis_d = 0.65,
    pelvis_y = 3.5,

    shoulder_sphere_r = 0.26,
    shoulder_y = 5.0,
    shoulder_x = 0.75,
    shoulder_sphere_inset = 0.05,

    shield_angle_deg = -24.0,
    shield_rot_y_deg = 10.0,
    shield_dx = 0.12,
    shield_dy = 0.32,
    shield_w = 0.70,
    shield_h = 0.55,
    shield_t = 0.12,
    shoulder_cube = 0.46,
    armature_w = 0.34,
    armature_h = 0.24,
    armature_d = 0.24,
    armature_offset = 0.22,

    plasma_radius = 0.14,
    plasma_length = 0.85,
    plasma_y = -0.18,
    plasma_z = 0.45,
    plasma_x = 0.32,
    plasma_x2 = 0.18,
    plasma_shroud_len_frac = 0.36,
    plasma_shroud_radius_scale = 1.9,
    plasma_shroud_taper = 0.9,
    plasma_shroud_offset_frac = -0.30,
    plasma_barrel_len_frac = 0.58,
    plasma_barrel_offset_frac = 0.17,
    plasma_muzzle_len_frac = 0.10,
    plasma_muzzle_radius_scale = 1.25,
    plasma_muzzle_tip_scale = 1.12,
    plasma_muzzle_offset_frac = 0.44,

    rocket_w = 0.78,
    rocket_h = 0.88,
    rocket_d = 0.68,
    rocket_z = 0.24,
    rocket_x = 0.32,

    torso_r = 0.82,
    torso_h = 2.10,
    torso_y = 3.65,

    neck_r = 0.22,
    neck_h = 0.38,
    neck_y = 4.80,

    head_w = 0.48,
    head_h = 0.52,
    head_z = 0.13,

    left_weapon = 0,  -- plasma
    right_weapon = 0, -- plasma
}
