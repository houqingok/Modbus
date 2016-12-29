#include "calc_rod_length.h"
#include "DataTypes.h"
#include "rtwtypes.h"



void calc_rod_length(float32 trans[3], float32 orient[3],float32 rod_attach_P[18],float32 servo_attach_B[18])
{
// define four translational matrix
    float32 length[6];
	float32 leg_vector[18];
    float32 rot_matrix[9];
    // calculate translation matrix
	float a = orient[0];
	float b = orient[1];
	float c = orient[2];

	rot_matrix[0]= cos(c)*cos(b);
	rot_matrix[3]= sin(c)*cos(b);
	rot_matrix[6]=-sin(b);

	rot_matrix[1]=-sin(c)*cos(a)+cos(c)*sin(b)*sin(a);
	rot_matrix[4]= cos(c)*cos(a)+sin(c)*sin(b)*sin(a);
	rot_matrix[7]= cos(b)*sin(a);

	rot_matrix[2]= sin(c)*sin(a)+cos(c)*sin(b)*cos(a);
	rot_matrix[5]=-cos(c)*sin(a)+sin(c)*sin(b)*cos(a);
	rot_matrix[8]= cos(b)*cos(a);

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
