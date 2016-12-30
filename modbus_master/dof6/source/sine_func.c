/*
 * sine_func.c
 *
 *  Created on: Dec 30, 2016
 *      Author: Cdz
 */
#include"lookup_table.h"
#include"sine_func.h"

float m_sin(float angle)
{

    return (float)(sine_table[(int)(500*angle/360)])/4096;
}
float m_cos(float angle)
{

    return (float)(sine_table[(int)(500*(90-angle)/360)])/4096;
}




