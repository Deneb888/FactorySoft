#pragma once

#include <vector>

class TempCtrlModel {

public:

	float dt, x, x0, y, xd, yd, drive;
	float init_temp;
	float c, c2, kp, ki, l, sige;
	float xdl1, xdl2, xdl3, xdl4;

	float h_bd, l_bd;
	float dn_temp, dn_time, an_temp, an_time, ex_temp, ex_time;
	float t;

	std::vector <double> v_x, v_t, v_s;

	TempCtrlModel();
	void simdt();
	float sim_step(float temp, float time, float ramp, float ov_temp, float ov_time);
	void set_init_temp(float);
};


