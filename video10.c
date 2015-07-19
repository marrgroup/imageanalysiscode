//read in video and do stuff...


/********************Output file looks like this************************************************/
/**(1) frame (2) x position (3) y position (4) short axis (5) long axis (6) angle (7) Deformation index**/

//headers
#include "cv.h"
#include "highgui.h"
#include <stdio.h>

//preallocations
CvCapture *capture = 0;
IplImage *image = 0, *gray = 0, *gray2 = 0, *save = 0, *original = 0, *testframe = 0, *roi = 0, *image04 = 0, *img = 0, *roi_s = 0, *roi_r = 0;// *cnt_img = 0;
//gray2 is the same as 'adaptivethreshold' image from other files - it's the destination of cvAdaptiveThreshold
int frames, c, x_l, x_r, y_u, y_d, red, blue, green, k, k_old, x_l_s, x_r_s, y_u_s, y_d_s, x_l_r, x_r_r, y_u_r, y_d_r;
int blocksize, subpix, param1, levels, k;
float x_l_r_pix, x_r_r_pix, x_l_s_pix, x_r_s_pix, x_l_pix, y_d_s_pix, y_d_r_pix, y_u_s_pix, y_u_r_pix, y_u_pix;
int i = 1;
float height, width, xpos, ypos, longaxis, shortaxis, angle, fps;
CvSeq* contour = 0;
CvSeq* points = 0;
float data[1000][8];//added 0
float data_old[1000][8];//added 0
int a = 0;
int b = 0;
int traj = 0;
FILE *fp[10000000];//added 0
char path[1000];//added 0
char string[256];

//special variables
CvBox2D box;
CvPoint center;
CvSize size;

//predefined variable
float pixelsize = 0.1395;//need to recalculate; in um/pixels
int stepsizex = 5/0.1395;//these values are all um/(um/pixels) to give pixels to the program
int stepsizey = 1./0.1395;
float stepsizelength = 1/0.1395;//this is for the delta(size) of cells - needs to be within this boundary to get detected as same cell
int cellsizexmin = 4.25/0.1395;
int cellsizexmax = 13.5/0.1395;
int cellsizeymin = 2.5/0.1395;
int cellsizeymax = 7/0.1395;







/*******************************************functions****************************************/



/*thresholding function: not practical (yet), because blocksize needs to always be an odd number.
As such, can't use a trackbar. Need something clever...*/






//Set ROI function

void set_roi(int argc)
	{
		cvZero(roi);//set 'gray2' to zeros
		cvCopy(gray,roi,0);//copy 'save' to 'gray2'
		cvLine(roi, cvPoint(x_l,0), cvPoint(x_l,height), cvScalar(250,0,0,0),1,8,0);
		cvLine(roi, cvPoint(0,y_u), cvPoint(width,y_u), cvScalar(250,0,0,0),1,8,0);
		cvLine(roi, cvPoint(x_r,0), cvPoint(x_r,height), cvScalar(250,0,0,0),1,8,0);
		cvLine(roi, cvPoint(0,y_d), cvPoint(width,y_d), cvScalar(250,0,0,0),1,8,0);
		cvShowImage("Set ROI",roi);
	}


//Set Stretch zone function

void set_stretch_roi(int argc)
	{
		cvZero(roi_s);//set 'gray2' to zeros
		cvCopy(gray,roi_s,0);//copy 'save' to 'gray2'
		cvLine(roi_s, cvPoint(x_l_s,0), cvPoint(x_l_s,height), cvScalar(250,0,0,0),1,8,0);
		cvLine(roi_s, cvPoint(0,y_u_s), cvPoint(width,y_u_s), cvScalar(250,0,0,0),1,8,0);
		cvLine(roi_s, cvPoint(x_r_s,0), cvPoint(x_r_s,height), cvScalar(250,0,0,0),1,8,0);
		cvLine(roi_s, cvPoint(0,y_d_s), cvPoint(width,y_d_s), cvScalar(250,0,0,0),1,8,0);
		cvShowImage("Set Stretch ROI",roi_s);
	}


//Set relax zone function

void set_relax_roi(int argc)
	{
		cvZero(roi_r);//set 'gray2' to zeros
		cvCopy(gray,roi_r,0);//copy 'save' to 'gray2'
		cvLine(roi_r, cvPoint(x_l_r,0), cvPoint(x_l_r,height), cvScalar(250,0,0,0),1,8,0);
		cvLine(roi_r, cvPoint(0,y_u_r), cvPoint(width,y_u_r), cvScalar(250,0,0,0),1,8,0);
		cvLine(roi_r, cvPoint(x_r_r,0), cvPoint(x_r_r,height), cvScalar(250,0,0,0),1,8,0);
		cvLine(roi_r, cvPoint(0,y_d_r), cvPoint(width,y_d_r), cvScalar(250,0,0,0),1,8,0);
		cvShowImage("Set Relax ROI",roi_r);
	}



//simple finding/drawing contours function from singlecontour.c

float contour_fit(int nothing)
	{
	    
		cvNamedWindow("Contour",CV_WINDOW_AUTOSIZE);
		cvNamedWindow("Original",CV_WINDOW_AUTOSIZE);
		
		CvMemStorage* storage;
	    CvSeq* contour;
	
	    // Create dynamic structure and sequence.
	    storage = cvCreateMemStorage(0);
	    contour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint) , storage);
	
	    // Threshold the source image. This needful for cvFindContours().
	    //cvThreshold( image03, image02, slider_pos, 255, CV_THRESH_BINARY );
	
	    // Find all contours.
	    cvFindContours( gray2, storage, &contour, sizeof(CvContour),
	                    CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cvPoint(0,0));
	
	    // Clear images. IPL use.
	    //cvZero(img);
	    //cvZero(gray);
		//cvZero(cnt_img);

		k = 1;

	    // This cycle draw all contours and approximate it by ellipses.
	    for(;contour;contour = contour->h_next)
	    {
	        int count = contour->total; // This is number point in contour
	        CvPoint center;
	        CvSize size;
	        CvBox2D box;
	
	        // Number point must be more than or equal to 6 (for cvFitEllipse_32f).
	        if( count < 6 )
	            continue;
	
	        CvMat* points_f = cvCreateMat( 1, count, CV_32FC2 );
	        CvMat points_i = cvMat( 1, count, CV_32SC2, points_f->data.ptr );
	        cvCvtSeqToArray( contour, points_f->data.ptr, CV_WHOLE_SEQ );
	        cvConvert( &points_i, points_f );
	
	        // Fits ellipse to current contour.
	        box = cvFitEllipse2( points_f );
			if(box.size.width > cellsizeymin && box.size.height > cellsizexmin && box.size.width < cellsizeymax && box.size.height < cellsizexmax)
			{
	        // Draw current contour on Iplimage 'original'
	        cvDrawContours(image04,contour,CV_RGB(255,255,255),CV_RGB(255,255,255),0,1,8,cvPoint(0,0));
	
	        // Convert ellipse data from float to integer representation.
	        center = cvPointFrom32f(box.center);
	        size.width = cvRound(box.size.width*0.5);
	        size.height = cvRound(box.size.height*0.5);
	

			//store data...
			data[k-1][0] = 0;
			data[k-1][1] = i;
			data[k-1][2] = box.center.x;
			data[k-1][3] = box.center.y;
			data[k-1][4] = box.size.width;
			data[k-1][5] = box.size.height;
			data[k-1][6] = box.angle;
			data[k-1][7] = 0;

			if(i == 1) //for first frame of video (no previous frame)
			{data_old[k-1][7] = 1;} //for continuing trajectory

			printf("Ellipse data nr. %d: i: %d, x %2.2f, y %2.2f, 1st axis: %2.2f \n", k, i, data[k-1][2]*pixelsize, data[k-1][3]*pixelsize, data[k-1][5]*pixelsize);


	        // Draw ellipse on Iplimage 'image04' -------> move to find_trajectories function to mimic Tobias' program. Also for coloring/tracking
	        /*cvEllipse(image04, center, size,
	                  -box.angle, 0, 360,
	                  CV_RGB(0,0,255), 1, CV_AA, 0);*/
	
	        cvReleaseMat(&points_f);
			k++;
			}
	    }
	
		return xpos;
		return ypos;
		return longaxis;
		return shortaxis;
		return angle;
	}






//Trajectory detection function...

float find_trajectories(char argv[2])
	{
		//this loop identifies continuing trajectories
		for (a = 1; a < k; a++)
			{
				for(b = 1; b < k_old; b++)
					{
						if(abs(data[a-1][2] - data_old[b-1][2]) < stepsizex && abs(data[a-1][3] - data_old[b-1][3]) < stepsizey && fabs(data[a-1][5] - data_old[b-1][5]) < stepsizelength)
							//if difference in x position is less than stepsizex
							//AND
							//if difference in y position is less than stepsizey
							//AND
							//if the difference in long axis (ellipse height??) is less than the stepsizelength ------> I think this is to make sure a different sized cell isn't detected instead...?
							{
								//printf("test continuing %d\n",i);
								//fp[(int)data_old[b-1][0]] = fopen(path,"a");//added**************************************
								fprintf(fp[(int)data_old[b-1][0]], "%d    %2.3f    %2.3f    %2.3f    %2.3f    %2.3f    %2.3f    0  0  0\n", i-1, data[a-1][2]*pixelsize, data[a-1][3]*pixelsize, data[a-1][4]*pixelsize, data[a-1][5]*pixelsize, data[a-1][6], (data[a-1][5] - data[a-1][4])/(data[a-1][5] + data[a-1][4]));
								data[a-1][7] = 1;
								data_old[b-1][7] = 1;
								data[a-1][0] = data_old[b-1][0];
								//fclose(fp[(int)data_old[b-1][0]]);//added**************************************************
								//printf("continuing %d\n",i);
							}
					}

				if (i >= frames-3) //end of movie
					{
						fclose(fp[(int)data_old[b-1][0]]);
					}
			}
			
		//this loop identifies new/beginning trajectories
		for(a = 1; a < k; a++)
			{
				if(data[a-1][7] == 0)
					{
					//printf("test new %d\n",i);
					traj = traj + 1;
					sprintf(path, "%s%d.txt", &argv[2], traj);
					printf("opening %s\n", path);
					fp[traj] = fopen(path,"w");
					fprintf(fp[traj], "%d    %2.3f    %2.3f    %2.3f    %2.3f    %2.3f    %2.3f    0  0  0\n", i-1, data[a-1][2]*pixelsize, data[a-1][3]*pixelsize, data[a-1][4]*pixelsize, data[a-1][5]*pixelsize, data[a-1][6], (data[a-1][5] - data[a-1][4])/(data[a-1][5] + data[a-1][4]));
					data[a-1][0] = traj;
					//fclose(fp[traj]);//added*****************************************
					}
				if(i >= frames-3) //end of movie
					{
						fclose(fp[traj]);
					}
			}

		//this loop identifies noncontinuous trajectories and closes file
		for(b = 1; b < k_old; b++)
			{
				if(data_old[b-1][7] == 0) //trajectory is not continued
					{
						sprintf(path, "%s%d.txt", &argv[2], (int)data_old[b-1][0]);
						//printf("test end %d\n", i);
						printf("closing %s\n",path);
						fclose(fp[(int)data_old[b-1][0]]);
					}
				if(i >= frames-3) //end of movie
					{
						fclose(fp[(int)data_old[b-1][0]]);
					}
			}

		//saves data of current frame for analysis of next frame, resets control parameter data[x][7] to 0
		for(a = 1; a < k; a++)
			{
				data_old[a-1][0] = data[a-1][0];
				data_old[a-1][1] = data[a-1][1];
				data_old[a-1][2] = data[a-1][2];
				data_old[a-1][3] = data[a-1][3];
				data_old[a-1][4] = data[a-1][4];
				data_old[a-1][5] = data[a-1][5];
				data_old[a-1][6] = data[a-1][6];
				data_old[a-1][7] = 0;

				box.center.x = data[a-1][2];
				box.center.y = data[a-1][3];
				box.size.width = data[a-1][4];
				box.size.height = data[a-1][5];
				box.angle = data[a-1][6];
				center = cvPointFrom32f(box.center);
				size.width = cvRound(box.size.width*0.5);
				size.height = cvRound(box.size.height*0.5);
				
				//draw ellipses to output pictures.
				//cvEllipse(image04, center, size,
	                  //-box.angle, 0, 360,
	                  //CV_RGB(0,0,255), 1, CV_AA, 0);

				//set colors
				red = ((int)data[a-1][0]-1)*80;
				blue = ((int)data[a-1][0]-1)*48 + 128;
				green = 128 - ((int)data[a-1][0]-1)*72;
					while(red > 255){red = red - 256;}
					while(blue > 255){blue = blue - 256;}
					while(green < 0){green = green + 256;}

				//redraw ellipses...? put on 'original'?
				cvEllipse(original, center, size,
						-box.angle, 0, 360,
						CV_RGB(red,green,blue), 1, CV_AA, 0);

				//lines showing ellipse details?
				cvLine(original, cvPoint(center.x - size.width*sin(box.angle*2*M_PI/360), center.y + size.height*cos(box.angle*2*M_PI/360)), cvPoint(center.x + size.width*sin(box.angle*2*M_PI/360), center.y - size.height*cos(box.angle*2*M_PI/360)), CV_RGB(red,green,blue),1,8,0);

			}
		k_old = k; //set contour for next frame

		//show images
		//cvShowImage("Contour", image04);
/************************************Uncomment this out to show the outlines again***************************************/
		cvShowImage("Original", original);
	}






/***************************************main function*****************************************/

int main(int argc, char** argv)
{
		//forced to include file name in command line
	if(argc !=3)
		{
		printf("add image file name and output file: ./video1 <video.avi> <output data file (no extension)>\n");
		return -1;
		}
	
	

	//Load video
	capture = cvCreateFileCapture(argv[1]);
	//capture is the structure containing the .avi file
	if(!capture)
	{
	printf("Could not initialize capturing\n");
	return -1;
	}
	

	//read values for ROI if available...
	FILE *readvalues = fopen("values.txt","r");
	if(!readvalues)
		{
		printf("no values found\n");
		}
	else
		{
		fscanf(readvalues, "%d  %d  %d  %d\n", &x_l, &y_u, &x_r, &y_d);
		fclose(readvalues);
		}

	FILE *readvaluesstretch = fopen("values_stretch.txt","r");
	if(!readvaluesstretch)
		{
		printf("no values found\n");
		}
	else
		{
		fscanf(readvaluesstretch, "%d  %d  %d  %d\n", &x_l_s, &y_u_s, &x_r_s, &y_d_s);
		fclose(readvaluesstretch);
		}

	FILE *readvaluesrelax = fopen("values_relax.txt","r");
	if(!readvaluesrelax)
		{
		printf("no values found\n");
		}
	else
		{
		fscanf(readvaluesrelax, "%d  %d  %d  %d\n", &x_l_r, &y_u_r, &x_r_r, &y_d_r);
		fclose(readvaluesrelax);
		}

	//get video info
	cvQueryFrame(capture);
	frames = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
	height = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
	width = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);

	//create the spot(?) for our IplImages?
	testframe = cvQueryFrame(capture);
	original = cvCreateImage(cvGetSize(testframe),8,3);
	gray = cvCreateImage(cvGetSize(testframe),8,1);
	gray2 = cvCreateImage(cvGetSize(testframe),8,1);
	save = cvCreateImage(cvGetSize(testframe),8,1);
	roi = cvCreateImage(cvGetSize(testframe),8,1);
	roi_s = cvCreateImage(cvGetSize(testframe),8,1);
	roi_r = cvCreateImage(cvGetSize(testframe),8,1);
	img = cvCreateImage(cvGetSize(testframe),8,1);
		//cnt_img = cvCreateImage(cvGetSize(testframe),8,1);
	image04 = cvCloneImage(testframe);

	//convert testframe to grayscale - store image in 'gray'
	cvCvtColor(testframe,gray,CV_BGR2GRAY);



	/**********All three ROI functions are here...***************/
	

	/****************Image processing ROI******************/

	//open window with trackbars...
	cvNamedWindow("Set ROI",0);
	cvResizeWindow("Set ROI",600,600);
	cvCreateTrackbar("x left position","Set ROI",&x_l,width,set_roi);
	cvCreateTrackbar("x right position","Set ROI",&x_r,width,set_roi);
	cvCreateTrackbar("y top position","Set ROI",&y_u,height,set_roi);
	cvCreateTrackbar("y down position","Set ROI",&y_d,height,set_roi);

	//call function...
	
	set_roi(0);
	int key = cvWaitKey(0);
	//waits until press a key to move on...

	//checks that ROI is valid
	if(x_r < x_l)
	{
	int x = x_r;
	x_r = x_l;
	x_l = x;
	}
	if(y_d < y_u)
	{
	int y = y_d;
	y_d = y_u;
	y_u = y;
	}
	if(x_r - x_l == 0 || y_u - y_d == 0)
	{
	printf("Invalid ROI\n");
	printf("    \n");
	return -1;
	}

	//Set ROI with values from lines...
	printf("ROI rectangle corner points: (%d/%d), (%d/%d)\n", x_l, y_u, x_r, y_d);
	printf("ROI width: %f um, height: %f um\n", (x_r-x_l)*pixelsize, (y_d-y_u)*pixelsize);
	x_l_pix = x_l*pixelsize;
	y_u_pix = y_u*pixelsize;
	printf("ROI 0 point: (%f, %f)\n", x_l_pix, y_u_pix);
	cvDestroyWindow("Set ROI");

	//opens and writes files with parameters of processed image (new video7.c)
	FILE *values = fopen("values.txt","w");
	fprintf(values, "%d  %d  %d  %d  %f  %f  %s\n", x_l, y_u, x_r, y_d, x_l_pix, y_u_pix, argv[1]);
	fclose(values);



/*****************************Stretch ROI**************************/

	//open window with trackbars...
	cvNamedWindow("Set Stretch ROI",0);
	cvResizeWindow("Set Stretch ROI",600,600);
	cvCreateTrackbar("x left position","Set Stretch ROI",&x_l_s,width,set_stretch_roi);
	cvCreateTrackbar("x right position","Set Stretch ROI",&x_r_s,width,set_stretch_roi);
	cvCreateTrackbar("y top position","Set Stretch ROI",&y_u_s,height,set_stretch_roi);
	cvCreateTrackbar("y down position","Set Stretch ROI",&y_d_s,height,set_stretch_roi);

	//call function...
	
	set_stretch_roi(0);
	int kye = cvWaitKey(0);
	//waits until press a key to move on...

	//checks that ROI is valid
	if(x_r_s < x_l_s)
	{
	int x_s = x_r_s;
	x_r_s = x_l_s;
	x_l_s = x_s;
	}
	if(y_d_s < y_u_s)
	{
	int y_s = y_d_s;
	y_d_s = y_u_s;
	y_u_s = y_s;
	}
	if(x_r_s - x_l_s == 0 || y_u_s - y_d_s == 0)
	{
	printf("Invalid ROI\n");
	printf("    \n");
	return -1;
	}

	//Set ROI with values from lines...
	printf("ROI stretch rectangle corner points: (%d/%d), (%d/%d)\n", x_l_s, y_u_s, x_r_s, y_d_s);
	printf("Stretch ROI width: %f um, height: %f um\n", (x_r_s-x_l_s)*pixelsize, (y_d_s-y_u_s)*pixelsize);
	x_l_s_pix = x_l_s*pixelsize;
	x_r_s_pix = x_r_s*pixelsize;
	y_d_s_pix = y_d_s*pixelsize;
	y_u_s_pix = y_u_s*pixelsize;
	printf("Stretch ROI x range: %f, %f\n", x_l_s_pix, x_r_s_pix);
	cvDestroyWindow("Set Stretch ROI");

	//opens and writes files with parameters of processed image (new video7.c)
	FILE *values_stretch = fopen("values_stretch.txt","w_r");
	fprintf(values_stretch, "%d  %d  %d  %d  %f  %f  %f  %f  %s\n", x_l_s, y_u_s, x_r_s, y_d_s, x_l_s_pix, x_r_s_pix, y_d_s_pix, y_u_s_pix, argv[1]);
	fclose(values_stretch);



/**********************Relax ROI*****************************/

	//open window with trackbars...
	cvNamedWindow("Set Relax ROI",0);
	cvResizeWindow("Set ROI",600,600);
	cvCreateTrackbar("x left position","Set Relax ROI",&x_l_r,width,set_relax_roi);
	cvCreateTrackbar("x right position","Set Relax ROI",&x_r_r,width,set_relax_roi);
	cvCreateTrackbar("y top position","Set Relax ROI",&y_u_r,height,set_relax_roi);
	cvCreateTrackbar("y down position","Set Relax ROI",&y_d_r,height,set_relax_roi);

	//call function...
	
	set_relax_roi(0);
	int yek_r = cvWaitKey(0);
	//waits until press a key to move on...

	//checks that ROI is valid
	if(x_r_r < x_l_r)
	{
	int x_r = x_r_r;
	x_r_r = x_l_r;
	x_l_r = x_r;
	}
	if(y_d_r < y_u_r)
	{
	int y_r = y_d_r;
	y_d_r = y_u_r;
	y_u_r = y_r;
	}
	if(x_r_r - x_l_r == 0 || y_u_r - y_d_r == 0)
	{
	printf("Invalid ROI\n");
	printf("    \n");
	return -1;
	}

	//Set ROI with values from lines...
	printf("ROI relax rectangle corner points: (%d/%d), (%d/%d)\n", x_l_r, y_u_r, x_r_r, y_d_r);
	printf("Relax ROI width: %f um, height: %f um\n", (x_r_r-x_l_r)*pixelsize, (y_d_r-y_u_r)*pixelsize);
	x_l_r_pix = x_l_r*pixelsize;
	x_r_r_pix = x_r_r*pixelsize;
	y_d_r_pix = y_d_r*pixelsize;
	y_u_r_pix = y_u_r*pixelsize;
	printf("Relax ROI x range: %f, %f\n", x_l_r_pix, x_r_r_pix);
	cvDestroyWindow("Set Relax ROI");

	//opens and writes files with parameters of processed image (new video7.c)
	FILE *values_relax = fopen("values_relax.txt","w_r");
	fprintf(values_relax, "%d  %d  %d  %d  %f  %f  %f  %f  %s\n", x_l_r, y_u_r, x_r_r, y_d_r, x_l_r_pix, x_r_r_pix, y_d_r_pix, y_u_r_pix, argv[1]);
	fclose(values_relax);



/*********************End of ROI functions...******************************/








	//Allocate spot for video to play...
	//cvNamedWindow("Original video",CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Thresholded video",CV_WINDOW_AUTOSIZE);
		//cvNamedWindow("Contours",CV_WINDOW_AUTOSIZE);
	//cvNamedWindow("Ellipses",CV_WINDOW_AUTOSIZE);
	cvMoveWindow("Thresholded video",0,400);
	cvMoveWindow("Original",400,0);
		//cvMoveWindow("Contours",400,400);
	//cvMoveWindow("Ellipses",600,600);

	printf("number of frames %d:\n", frames);
	printf("input param1 for cvSmooth function (7):\n");
	scanf("%d", &param1);
	printf("input number of pixels for cvAdaptiveThreshold (91):\n");
	scanf("%d", &blocksize);
	printf("input number of pixels to subtract from image (-1):\n");
	scanf("%d", &subpix);
	printf("input max_level for cvDrawContours (2):\n");
	scanf("%d", &levels);

	//set ROI to the images - this is for checking the contour fit - add in later
	//cvCopy(testframe,original,0);

	//open file for data output (video7.c addition)
	sprintf(path, "%s%d.txt", argv[2],0);
	//FILE *singal = fopen(path, "w");

	//for loop to play video

	for(i = 1; i < frames-2; i++)
	//while(1)
		{
		cvZero(gray2);
		cvZero(gray);
		cvZero(original);
			//cvZero(cnt_img);
		cvZero(img);
		cvZero(image04);
		//reset images

		testframe = cvQueryFrame(capture);
		//grab a frame from 'capture'

		original = cvCreateImage(cvGetSize(testframe),8,3);
		gray = cvCreateImage(cvGetSize(testframe),8,1);
		gray2 = cvCreateImage(cvGetSize(testframe),8,1);
		save = cvCreateImage(cvGetSize(testframe),8,1);
		roi = cvCreateImage(cvGetSize(testframe),8,1);
		img = cvCreateImage(cvGetSize(testframe),8,1);
		image04 = cvCloneImage(testframe);
		//reallocate spots for all images - same size this way

		cvCvtColor(testframe,gray,CV_BGR2GRAY);
		//convert to grayscale -> 'gray'
		cvCvtColor(gray,original,CV_GRAY2BGR);
		
		cvSmooth(gray,gray,CV_BLUR,param1,0,0,0);
		cvAdaptiveThreshold(gray,gray2,255,CV_ADAPTIVE_THRESH_GAUSSIAN_C,CV_THRESH_BINARY,blocksize,subpix);
		cvNot(gray2,gray2);
		cvCopy(gray2,img,0);
		//threshold 'gray' -> 'gray2' is thresholded image
		if (!testframe) break;

		cvSetImageROI(gray2, cvRect(x_l, y_u, x_r-x_l, y_d-y_u));
		cvSetImageROI(original, cvRect(x_l, y_u, x_r-x_l, y_d-y_u));
			//cvSetImageROI(cnt_img, cvRect(x_l, y_u, x_r-x_l, y_d-y_u));
		cvSetImageROI(image04, cvRect(x_l, y_u, x_r-x_l, y_d-y_u));
		cvSetImageROI(img, cvRect(x_l, y_u, x_r-x_l, y_d-y_u));



		contour_fit(0);
			//it's important to set roi before running functions, as done here - that way the data only comes from the roi image, not the original

		find_trajectories(argv[2]);
		
		//cvShowImage("Original video",original);
		cvShowImage("Thresholded video",img);/*******************************comment this back in to see threshold*********/
			//cvShowImage("Contours",cnt_img);
		//cvShowImage("Ellipses",image04);
		
		char c = cvWaitKey(2);
		if (c == 27) break;
		//if press "Esc" the program will end
//getchar();

	}

	
	




	
	
	cvDestroyWindow("Thresholded video");
	
	//cvDestroyWindow("Original video");
		//cvDestroyWindow("Contours");
	//cvDestroyWindow("Ellipses");
	cvReleaseImage(&save);
	cvReleaseImage(&original);
	cvReleaseImage(&testframe);
	cvReleaseImage(&gray);
	cvReleaseImage(&gray2);
	cvReleaseImage(&img);
	cvReleaseImage(&image04);
		//cvReleaseImage(&cnt_img);
	
	cvReleaseCapture(&capture);

	return 0;
}
