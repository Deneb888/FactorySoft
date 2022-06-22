// TempCtrlModel.cpp


#include "stdafx.h"
#include "TempCtrlModel.h"

//=============

TempCtrlModel::TempCtrlModel() 
{
	dt = 0.035f;

	c = 12; c2 = 1.5; l = 0.3;

	kp =300; ki = 0.2;

	h_bd = 256; l_bd = -90;

	sige = 0;

	set_init_temp(30);

/*	dn_temp = 95;
	dn_time = 20;
	an_temp = 65;
	an_time = 30;
	ex_temp = 72;
	ex_time = 20;
*/
	t = 0;
}

void TempCtrlModel::set_init_temp(float init_temp) 
{
//	init_temp = 30;

	y = init_temp;
	x = init_temp;

	x0 = init_temp;

	xdl1 = xdl2 = xdl3 = xdl4 = init_temp;
}

void TempCtrlModel::simdt() {

	sige = sige + (x0 - xdl4);
	drive = -kp * xdl4 + kp * x0 + ki * sige;

	if (drive > h_bd)
		drive = h_bd;
	else if (drive < l_bd)
		drive = l_bd;

	xd = (-l*x + drive) / c;
	x = x + xd * dt;

	xdl4 = xdl3;
	xdl3 = xdl2;
	xdl2 = xdl1;
	xdl1 = x;

	yd = (-y + x) / c2;
	y = y + yd * dt;

	t += dt;

	v_x.push_back(y);
	v_t.push_back(t);
	v_s.push_back(x0);
}

float TempCtrlModel::sim_step(float set_point, float time_period, float ramp_speed, float ov_temp, float ov_time)
{
	float timer = 0;
	float diff;
	float ramp_time = 0;
	float timeout_time = 1000;
	int target = 0;

	time_period *= 1.1;
	time_period += 3;

	int dir = 0;	// up
	if (set_point < x)
		dir = 1;	// down

	float ovs = 0;
	if (!dir)
		ovs = ov_temp;
	else
		ovs = -ov_temp;

	diff = abs(set_point + ovs - x0);

	if (ramp_speed > 0) {
		ramp_time = diff / ramp_speed;
	}

	while (timer < timeout_time) {
		if (timer < ramp_time) {
			if (!dir) {
				x0 += ramp_speed * dt;
			}
			else {
				x0 -= ramp_speed * dt;
			}
		}
		else if (timer < ramp_time + ov_time) {
			x0 = set_point + ovs;
		}
		else x0 = set_point;

		simdt();
		timer += dt;
		diff = abs(x - set_point);
		if (diff < 0.5 && !target) {
			timeout_time = timer + time_period;
			target = 1;
		}
	}

	return timer;
}


