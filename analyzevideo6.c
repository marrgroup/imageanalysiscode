
//***************************************************************************************************************
//*******************************FULL ANALYSIS PROGRAM******************************************************
//************************** analyzes a set of trajectories of cells in flow ********************************
//********************************************05.23.2012******************************************************
//********** gets parameters from laser signal, averages cell stretching, compares parameters*****************
//*********************************************INPUT DATA STRUCTURE*******************************************
//*** /**(1) frame (2) x position (3) y position (4) short axis (5) long axis (6) angle (7) Deformation index**/ ***
//*********************************************OUTPUT DATA STRUCTURE*******************************************
//**** INPUT FILE + 8: Time 9: sine fit long axis, 10: sine fit short axis **********************************

//* command: ./flowanalyze <datafile(no extension)> number of trajectories <outputfile.txt> freq  ***
//***************** analyzes all files <datafile(n).txt> in directory ***************************************
//note that the commented out printf functions are checks for when I was building the program


#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <ctype.h>
#include <math.h>

int frames, stretch_frames, relax_frames, pos, j, numtraj, n, status, stretch_status, relax_status, i, correct, cells, abscells, k;
int x_l, x_r, y_u, y_d, x_l_s, x_r_s, y_u_s, y_d_s, x_l_r, x_r_r, y_u_r, y_d_r;
float x_l_s_pix, x_l_r_pix, x_r_s_pix, x_r_r_pix, x_l_pix, sum_stretch_major, sum_stretch_minor, sum_relax_major, sum_relax_minor, def_major, def_minor, y_d_s_pix, y_d_r_pix, y_u_s_pix, y_u_r_pix, y_u_pix;
float maxRe, freq, phasecorrect, setfreq, temp, temp3, temp4, trash, shift, average_stretch_major, average_stretch_minor, average_relax_major, average_relax_minor, stretch_axis_ratio, relax_axis_ratio;
CvBox2D box;
double area;
char path[1000];
char path2[1000];//each "path#" is just the string storage for writing to txt files
char path3[1000];
char path4[1000];
char path5[1000];
char path6[1000];
CvContour *ct6r;
int fps;


int pixelsize = 0.1395;
int minlength = 45;
int show = 0;
int bars = 15;
int smoothframes = 10; // even number

//comment these out if want to input fps and traj information
int fps = 1000;






int main(int argc, char **argv)
{

abscells = 0;
//column = 4;

//check arg
	if(argc != 3)
	{
	printf("Please add input file, # of trajectories and output file name plus used frequency\n");
	printf("only use the stem of the filename, <n>.txt will be added by the program\n"); 
	printf("./analysis <datafile> <# of trajectories>\n");
	return -1;
	}
//FILE *results = fopen(argv[3], "w");//this opens file called whatever the 'outputfile' text is named in command window

/***********************Comment these back in after finishing one type of video where fps and trajectory number are set******/
//printf("Please type the video speed (frames per second): \n");
//scanf("%d", &fps);

//printf("Type how many frames are required to make a full trajectory (20): \n");
//scanf("%d", &minlength);

//read ROI values from detection program
FILE *readvalues = fopen("values.txt","r");
	if(!readvalues)
		{
		printf("no values found\n");
		}
	else
		{
		fscanf(readvalues, "%d  %d  %d  %d  %f %f\n", &x_l, &y_u, &x_r, &y_d, &x_l_pix, &y_u_pix);
		fclose(readvalues);
		}

	FILE *readvaluesstretch = fopen("values_stretch.txt","r");
	if(!readvaluesstretch)
		{
		printf("no values found\n");
		}
	else
		{
		fscanf(readvaluesstretch, "%d  %d  %d  %d %f %f%f %f\n", &x_l_s, &y_u_s, &x_r_s, &y_d_s, &x_l_s_pix, &x_r_s_pix, &y_d_s_pix, &y_u_s_pix);
		fclose(readvaluesstretch);
		}

	FILE *readvaluesrelax = fopen("values_relax.txt","r");
	if(!readvaluesrelax)
		{
		printf("no values found\n");
		}
	else
		{
		fscanf(readvaluesrelax, "%d  %d  %d  %d %f %f%f %f \n", &x_l_r, &y_u_r, &x_r_r, &y_d_r, &x_l_r_pix, &x_r_r_pix, &y_d_r_pix, &y_u_r_pix);
		fclose(readvaluesrelax);
		}




// start detection of single trajectories
numtraj = atof(argv[2]);
FILE *fp[numtraj];
FILE *fp2[numtraj];
FILE *fp3[numtraj];
FILE *fp4[numtraj];
FILE *fp5[numtraj];
FILE *fp6[numtraj];
float final_data[numtraj];//[7]
cells = 0;

sprintf(path6, "%soutput.txt", argv[1]);
FILE *output = fopen( path6 ,"w");
//fprintf(output, "Traj     Avg_Maj_S     Avg_Maj_R     Avg_Min_S     Avg_Maj_R     Def_Maj     Def_Min     Stretch Maj/Min     Relax Maj/Min\n"); /******************pull this out to eliminate text from output*************************/
fclose(output);//create output file %s_output.txt to save each trajectory summary information

//(for loop) for all trajectories
for(n = 1; n <= numtraj; n++)
{
	sprintf(path, "%s%d.txt", argv[1], n);//writes results to 'path' from text file

	fp[n] = fopen( path ,"r");//opens a file 'path' and reads it from start
	frames = 0;
	status = 0;
	while (status != EOF)
	{
		status = fscanf(fp[n], "%f\n", &temp);//reads ALL data from 'fp[n]' and stores it in temp
		frames++;//this 'while' loop will cycle (frames)*10 because there are 10 columns of data in each txt file
				 //so it reads each int or float (stores as float for later) in temp
				//is this just a strange way to count frames? because I don't see what else it would be used for...
	}
	frames = frames/10;
	printf("Traj. %d: %d frames\n", n,frames);
	fclose(fp[n]);

	if(frames < minlength)
	{
		remove(path);
	}
	else
	{

//******************************** read data from cell detection program *****************************************
fp2[n] = fopen( path ,"r");// "path" can be referred to by the fp2[n] file pointer
float data[8][frames];//allocate space for data array

sprintf(path2, "stretch_%d.txt", n);// content is stored as a string in location path2????
fp3[n] = fopen(path2, "w");
//open file for saving stretch ROI for each trajectory

sprintf(path3, "relax_%d.txt", n);
fp4[n] = fopen(path3, "w");
//open file for saving stretch ROI for each trajectory


for(i = 1; i <= frames; i++)
{

	//all data in fp2[n] is written to locations of the data[][] array or trash for the 0's
	fscanf(fp2[n], "%f	  %f    %f    %f    %f    %f   %f   %f   %f   %f\n", &data[0][i-1], &data[1][i-1], &data[2][i-1], &data[3][i-1], &data[4][i-1], &data[5][i-1], &data[6][i-1], &trash, &trash, &trash);

	//data: 0=frame, 1=xpos, 2=ypos, 3=min axis, 4=maj axis, 5=angle, 6=DI

	data[7][i-1] = data[0][i-1]/fps;
	//time(s)

	fprintf(fp2[n], "%f   %f   %f   %f   %f   %f   %f   %f   0   0\n", data[0][i-1], data[1][i-1], data[2][i-1], data[3][i-1], data[4][i-1], data[5][i-1], data[6][i-1], data[7][i-1]);




		//stretch data
		if (data[1][i-1] >= x_l_s_pix-x_l_pix && data[1][i-1] <= x_r_s_pix-x_l_pix && data[2][i-1] >= y_u_s_pix-y_u_pix && data[2][i-1] <= y_d_s_pix-y_u_pix)//need to subtract LHS point of ROI, data collection begins at "0" (calls corner of ROI (0,0)), but our ROI still refers to x-values relative to full image
		{
			//printf("%f is the x position for stretched maj axis diameter: %f\n",data[1][i-1],data[4][i-1]);
			fprintf(fp3[n], "%f     %f     %f     %f     %f\n", data[0][i-1]/fps, data[1][i-1], data[3][i-1], data[4][i-1], data[6][i-1]); 
		
			//data: 0=time, 1=xpos, 3=min diameter, 4=maj diameter, 6=DI
		}



//relaxation data
		if (data[1][i-1] >= x_l_r_pix-x_l_pix && data[1][i-1] <= x_r_r_pix-x_l_pix && data[2][i-1] >= y_u_r_pix-y_u_pix && data[2][i-1] <= y_d_r_pix-y_u_pix)//need to subtract LHS point of ROI, data collection begins at "0" (calls corner of ROI (0,0)), but our ROI still refers to x-values relative to full image
		{
			//printf("%f is the x position for relaxed maj axis diameter: %f\n",data[1][i-1],data[4][i-1]);
			fprintf(fp4[n], "%f     %f     %f     %f     %f\n", data[0][i-1]/fps, data[1][i-1], data[3][i-1], data[4][i-1], data[6][i-1]); 
			
			//data: 0=time, 1=xpos, 3=min diameter, 4=maj diameter, 6=DI

		
		}




}

fclose(fp2[n]);
fclose(fp3[n]);
fclose(fp4[n]);



	fp3[n] = fopen( path2 ,"r");//opens a file 'path' and reads it from start
			stretch_frames = 0;
			stretch_status = 0;
				while (stretch_status != EOF)
				{
					stretch_status = fscanf(fp3[n], "%f\n", &temp3);//identical while loop as above, to count frames in file
					stretch_frames++;
				}
			stretch_frames = stretch_frames/5;//only 5 columns of data now, this may be part of the problem?
	fclose(fp3[n]);


	fp4[n] = fopen( path3 ,"r");//opens a file 'path' and reads it from start
			relax_frames = 0;
			relax_status = 0;
				while (relax_status != EOF)
				{
					relax_status = fscanf(fp4[n], "%f\n", &temp4);//identical while loop as above, to count frames in file
					relax_frames++;
				}
			relax_frames = relax_frames/5;
	fclose(fp4[n]);

//printf("Stretch Traj. %d: %d frames\n", n,stretch_frames);
//printf("Relax Traj. %d: %d frames\n", n,relax_frames);






/******************************Stretch averaging**************************/
sprintf(path4, "stretch_%d.txt", n);
fp5[n] = fopen( path4 ,"r");
float stretch_data[5][stretch_frames];//allocate space for data array


float stretch_major[stretch_frames];
float stretch_minor[stretch_frames];
sum_stretch_major = 0;
sum_stretch_minor = 0;
average_stretch_major = 0;
average_stretch_minor = 0;
//reset variables

for (j = 1; j <=stretch_frames; j++)
{
	
	fscanf(fp5[n], "%f	  %f    %f    %f    %f\n", &stretch_data[0][j-1], &stretch_data[1][j-1], &stretch_data[2][j-1], &stretch_data[3][j-1], &stretch_data[4][j-1]);

	//data: 0=frame, 1=xpos, 2=min axis, 3=maj axis, 4=DI

	stretch_major[j] = stretch_data[3][j-1];
	stretch_minor[j] = stretch_data[2][j-1];

	sum_stretch_major = sum_stretch_major + stretch_major[j];
	sum_stretch_minor = sum_stretch_minor + stretch_minor[j];

}


average_stretch_major = sum_stretch_major/stretch_frames;
average_stretch_minor = sum_stretch_minor/stretch_frames;


//printf("sum stretch major axis: %f\n", sum_stretch_major);
//printf("sum stretch minor axis: %f\n", sum_stretch_minor);
printf("avg stretch major axis: %f\n", average_stretch_major);
printf("avg stretch minor axis: %f\n", average_stretch_minor);



/******************************Relax averaging**************************/
sprintf(path5, "relax_%d.txt", n);
fp6[n] = fopen( path5 ,"r");
float relax_data[5][relax_frames];//allocate space for data array


float relax_major[relax_frames];
float relax_minor[relax_frames];
sum_relax_major = 0;
sum_relax_minor = 0;
average_relax_major = 0;
average_relax_minor = 0;
//reset variables

for (k = 1; k <=relax_frames; k++)
{
	
	fscanf(fp6[n], "%f	  %f    %f    %f    %f\n", &relax_data[0][k-1], &relax_data[1][k-1], &relax_data[2][k-1], &relax_data[3][k-1], &relax_data[4][k-1]);

	//data: 0=frame, 1=xpos, 2=min axis, 3=maj axis, 4=DI

	relax_major[k] = relax_data[3][k-1];
	relax_minor[k] = relax_data[2][k-1];

	sum_relax_major = sum_relax_major + relax_major[k];
	sum_relax_minor = sum_relax_minor + relax_minor[k];

}


average_relax_major = sum_relax_major/relax_frames;
average_relax_minor = sum_relax_minor/relax_frames;


//printf("sum relax major axis: %f\n", sum_relax_major);
//printf("sum relax minor axis: %f\n", sum_relax_minor);
printf("avg relax major axis: %f\n", average_relax_major);
printf("avg relax minor axis: %f\n", average_relax_minor);


/*****************************Deliverable parameters*************************************/

def_major = (average_stretch_major - average_relax_major)/(average_relax_major)*100;
def_minor = (average_stretch_minor - average_relax_minor)/(average_relax_minor)*100;

printf("percent deformation major = %f\n", def_major);
printf("percent deformation minor = %f\n", def_minor);

stretch_axis_ratio = average_stretch_major/average_stretch_minor;
relax_axis_ratio = average_relax_major/average_relax_minor;

float final_data[numtraj];//allocate space for data array

final_data[n] = def_major;
//printf("major def: %f\n", final_data[n]);
output = fopen(path6, "a");
fprintf(output, "%d     %f     %f     %f     %f     %f     %f     %f     %f\n", n, average_stretch_major, average_relax_major, average_stretch_minor, average_relax_minor, def_major, def_minor, stretch_axis_ratio, relax_axis_ratio);
//0=traj, 1=avg maj stretch, 2=avg maj relax, 3=avg min stretch, 4=avg min relax, 5=%maj, 6=%min, 7=ratio
fclose(output);








/*relax data
for(i = 1; i <= frames; i++)
{
	if(data[1][i-1] >= x_l_r_pix && data[1][i-1] <= x_r_r_pix)
	{
		printf("%f is the x position for relaxed maj axis diameter: %f\n", data[1][i-1], data[4][i-1]);
	}
}

printf("first frame of %d is %f\n", n, data[0][0]);//data[column][row]
printf("the corresponding time is %f\n",data[7][0]);
*/




/*
FILE *writevalues2 = fopen( argv[1] ,"w");
for(i = 1; i <= frames; i++)
{
fprintf(writevalues2, "%f   %f   %f   %f   %f   %f   %f   %f   0   0\n", data[0][i-1], data[1][i-1], data[2][i-1], data[3][i-1], data[4][i-1], data[5][i-1], data[6][i-1], data[7][i-1]);
}
fclose(writevalues2);
*/








}//end if longer than minlength

}//end for loop over all trajectories (n)

printf("The stretching ROI is from (%f, %f) to (%f, %f) \n", x_l_s_pix-x_l_pix, y_u_s_pix-y_u_pix, x_r_s_pix-x_l_pix, y_d_s_pix-y_u_pix);
printf("The relaxation ROI is from (%f, %f) to (%f, %f) \n", x_l_r_pix-x_l_pix, y_u_r_pix-y_u_pix, x_r_r_pix-x_l_pix, y_d_r_pix-y_u_pix);

/*for (n = 1; n <= numtraj; n++)
{
	printf("All major def: %f\n", final_data[n]);
}*/

return 0;
}

