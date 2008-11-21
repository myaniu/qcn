#include <stdio.h>
#include <math.h>

int main()
{
   float eq_mag, sensor_type;
   printf("Enter magnitude of the earthquake: \n");
   scanf("%f", &eq_mag);
   
   printf("Enter sensor type: (1=Thinkpad, 2=Mac, 3=JW, 4=MN) \n");
   scanf("%f", &sensor_type); 
   
   float temp1 = eq_mag-(5.4-sqrt(sensor_type-1));
   float max_dist = powf( 2. , temp1);
   printf("Maximum distance %f \n", max_dist);
}
