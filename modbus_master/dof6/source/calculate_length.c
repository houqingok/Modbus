#include "calc_rod_length.h"

extern float32 rod_attach_P[18];
extern float32 servo_attach_B[18];
extern float32 length[6];

void calc_rod_length(float32 trans[3],float32 orient[3])
{
// define four translational matrix
	float32 leg_vector[18];
    float32 rot_matrix[9];
    // calculate translation matrix
	float a = orient[0];
	float b = orient[1];
	float c = orient[2];

	rot_matrix[0]= m_cos(c)*m_cos(b);
	rot_matrix[3]= m_sin(c)*m_cos(b);
	rot_matrix[6]=-m_sin(b);

	rot_matrix[1]=-m_sin(c)*m_cos(a)+m_cos(c)*m_sin(b)*m_sin(a);
	rot_matrix[4]= m_cos(c)*m_cos(a)+m_sin(c)*m_sin(b)*m_sin(a);
	rot_matrix[7]= m_cos(b)*m_sin(a);

	rot_matrix[2]= m_sin(c)*m_sin(a)+m_cos(c)*m_sin(b)*m_cos(a);
	rot_matrix[5]=-m_cos(c)*m_sin(a)+m_sin(c)*m_sin(b)*m_cos(a);
	rot_matrix[8]= m_cos(b)*m_cos(a);

// calculate coordinates qi of anchor points pi with respect to the base reference frame
	int i,j,k;

	// matrix multiplication
	float32 Rbpi[18];
    float32 temp=0;
    for(i=0;i<3;i++)
    {
    	for(j=0;j<6;j++)
    	{

    		for(k=0;k<3;k++)
    		    {
    			    temp+=rot_matrix[3*i+k]*rod_attach_P[6*k+j];
    		    }

    		Rbpi[6*i+j] = temp;
    		temp=0;

    	}

   }
// calculate six leg vectors
for(j=0;j<6;j++)
{
	for(i=0;i<3;i++)
	{
		leg_vector[6*i+j] = trans[i] + Rbpi[6*i+j] -servo_attach_B[6*i+j] ;
	}

}

// calculate leg length
for(j=0;j<6;j++)
    {
        length[j]=sqrt(pow(leg_vector[j],2)+pow(leg_vector[j+6],2)+pow(leg_vector[j+2*6],2));
    }
}
