#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <iostream>
#include <time.h>

#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>


#include <dirent.h>
#include <string.h>

using namespace dlib;
using namespace std;

/* 函数声明 */
int *face_location(char *imgFile);
bool liveness_detection(char *DeepFile, int rec_face[4]); 
int processFileList(char *basePath); 

const int IMG_HEIGHT =  720;
const int IMG_WIDTH =  1280;

int main(void)
{
    char *basePath = "/home/zhoujie/cProject/dlib_test/face";
    processFileList(basePath);
    return 0;
}

int processFileList(char *basePath)
{
    DIR *dir;
    struct dirent *ptr; 
    char base[30]; 
    char title[5];

    char *p=".raw"; //需要的子串;
    char *padd="/";

    char *rawl="raw_";
    char *rawr="_frontface.raw";
    char *jpgr="_IR_frontface.jpg";

	int image_num=0;
	int image_right=0;

    char title_raw[50];
    char title_jpg[50];
    char file_path_raw[120];
    char file_path_jpg[120];

    if ((dir=opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        exit(1);
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        strcpy(base, ptr->d_name);
        // printf("%s\n",base); 


        if(strstr(base,p)) 
        {
            memset(title, '\0', sizeof(title));
            title[0] =base[4];
            title[1] =base[5];
            title[2] =base[6];
            title[3] =base[7];
            printf("\nNo.image : %s\n",title);

            strcpy(file_path_raw, basePath);
            strcat(file_path_raw, padd);
            strcpy(file_path_jpg, file_path_raw);
            // printf("%s\n",file_path_raw);

            strcpy(title_raw, rawl);
            strcat(title_raw, title);
            strcat(title_raw, rawr);
            // printf("%s\n",title_raw);
            strcpy(title_jpg, title);
            strcat(title_jpg, jpgr);
            // printf("%s\n",title_jpg);

            strcat(file_path_raw, title_raw);
            strcat(file_path_jpg, title_jpg);

            printf("%s\n",file_path_raw); 
            printf("%s\n",file_path_jpg); 

            int *rec_face;
            rec_face = face_location(file_path_jpg);

            //深度图与红外图是水平翻转的
	        rec_face[0] = IMG_WIDTH - rec_face[0] -rec_face[2];

            bool IS_FACE;
            /* 调用函数判断是否为活体 */
            IS_FACE = liveness_detection( file_path_raw, rec_face);
            printf("RESULT : %d\n", IS_FACE);

			if (IS_FACE ==  true)
			{
				image_right +=  1;
			}

			image_num +=  1;

            delete rec_face;

        }  
    }

	printf("face image num ： %d\n", image_num);
	printf("detect right num : %d\n", image_right);

    closedir(dir);
	
}


/* 函数 输出人脸位置 */
int *face_location(char* imgFile)
{  
    int *rec_face = new int[4];

    frontal_face_detector detector = get_frontal_face_detector();

    cout << "processing image " << imgFile << endl;

    clock_t start,finish;
    double totaltime;
    start=clock();

    array2d<unsigned char> img;
    load_image(img, imgFile);
    
    std::vector<rectangle> dets = detector(img);

    cout << "Number of faces detected: " << dets.size() << endl;

    rec_face[0] = dets[0].left();
    rec_face[1] = dets[0].top();
    rec_face[2] = dets[0].right() - dets[0].left() + 1;
    rec_face[3] = dets[0].bottom() - dets[0].top() + 1;

    finish=clock();
    totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
    cout<<"此程序的运行时间为"<<totaltime<<"秒！"<<endl;

    // delete rec_face;

    return  rec_face;
}

/* 函数判断是否为活体 */
bool liveness_detection(char *DeepFile, int rec_face[4])
{
    const int ITER = 10000; // 随机取点次数
    const float PLANE_OR_NOT = 0.1; // 判断是否为平面的分界线
	const int SIGMA = 1;
    typedef unsigned short UNIT16;
	
	// 从.raw读取二进制16位数据到MatDATA
	UNIT16 MatDATA[IMG_HEIGHT*IMG_WIDTH];
	FILE *fp = NULL;
	fp = fopen( DeepFile, "rb" );
    fread(MatDATA,sizeof(UNIT16),IMG_HEIGHT*IMG_WIDTH,fp);
	fclose(fp);
	
	int n = 0;
	int i,j;
	// // DeepDATA三行分别为深度图行数，列数，深度信息
	// int DeepDATA[3][IMG_HEIGHT*IMG_WIDTH];
	// for(i=1;i< IMG_HEIGHT+1 ;i++)
	// 	{
	// 		for(j=1;j< IMG_WIDTH+1 ;j++) 
	// 		{ 
	// 			DeepDATA[0][n] = i;
	// 			DeepDATA[1][n] = j;
	// 			DeepDATA[2][n] = MatDATA[n];
	// 			n += 1;
	// 		} 
	// 	} 

    // n = 0; 
	int COL = rec_face[0],ROW = rec_face[1],FACE_WIDTH = rec_face[2],FACE_HEIGHT = rec_face[3]; //位置信息
	// txt :157 66 172 198 , 取行66：66+198,列取157：157+172
	int faceno0_num = FACE_HEIGHT*FACE_WIDTH -1; 
	int FaceDATA[3][160000];
	n = 0;
	for(i = 1;i< FACE_HEIGHT+1;i++)
		{
			for(j= 1;j< FACE_WIDTH+1;j++) 
			{ 
				if (MatDATA[IMG_WIDTH*(ROW+i-2)+COL+j-2] == 0)
				{
					faceno0_num -= 1; // 非零深度点个数为 faceno0_num+1
					continue;
				}
				FaceDATA[1][n] = i;
				FaceDATA[0][n] = j; 
				FaceDATA[2][n] = MatDATA[IMG_WIDTH*(ROW+i-2)+COL+j-2];
				n += 1;
			} 
		} 
	// int test = 0;  
	// printf("%d,%d,%d,%d\n",test,FaceDATA[0][test],FaceDATA[1][test],FaceDATA[2][test]);	
	
	int pretotal = 0;  // 符合拟合模型的数据的个数
	int x[3],y[3],z[3];  // 随机取三个点 
	srand((unsigned)time(NULL));
	float a,b,c;  // 拟合平面方程 z=ax+by+c
	// float besta,bestb,bestc;  // 最佳参数
	int rand_num[3];
	float check,distance;
	int total = 0;
	for(i = 0; i < ITER; i++)
	{
		do{
			rand_num[0] = std::rand()%faceno0_num; 
			rand_num[1] = std::rand()%faceno0_num; 
			rand_num[2] = std::rand()%faceno0_num; 
		}while(rand_num[0] == rand_num[1] || rand_num[0] == rand_num[2] || rand_num[1] == rand_num[2]);
		for(n = 0; n < 3; n++ )
		{
			x[n] = FaceDATA[0][rand_num[n]];
			y[n] = FaceDATA[1][rand_num[n]];
			z[n] = FaceDATA[2][rand_num[n]];
			// printf("%d,%d,%d,%d\n", x[n],y[n],z[n],n);
		}
		check = (x[0]-x[1])*(y[0]-y[2]) - (x[0]-x[2])*(y[0]-y[1]);
		if ( check == 0)  // 防止提示浮点数例外 (核心已转储)
		{
			i -= 1;
			continue;
		}
		a = ( (z[0]-z[1])*(y[0]-y[2]) - (z[0]-z[2])*(y[0]-y[1]) )/( (x[0]-x[1])*(y[0]-y[2]) - (x[0]-x[2])*(y[0]-y[1]) );
        if (y[0] == y[2])  // 防止提示浮点数例外 (核心已转储)
		{
			i -= 1;
			continue;
		}
		b = ((z[0] - z[2]) - a * (x[0] - x[2]))/(y[0]-y[2]);
        c = z[0]- a * x[0] - b * y[0];
		// printf("%f,%f,%f\n",a,b,c);
		total = 0;
		for(n = 0; n < faceno0_num +1 ; n++ )
		{
			distance = fabs(a*FaceDATA[0][n] + b*FaceDATA[1][n] - 1*FaceDATA[2][n] + c*1);
			if (distance < SIGMA)
			{
				total +=1;
			}
		}
		// printf("%d,%f,%d\n",i,distance,total);
		if (total > pretotal)  // 找到符合拟合平面数据最多的拟合平面
        {
			pretotal=total;
			// besta = a;
			// bestb = b;
			// bestc = c;
		}
	}
	float pretotal_ary = pretotal *1.0/ faceno0_num ;
	printf("%d,%f\n", pretotal,pretotal_ary);
	bool IS_FACE;

    if (pretotal_ary < PLANE_OR_NOT)
	{
		IS_FACE =  true;
		// printf("是人脸");
	}
	else
	{
		IS_FACE = false;
		// printf("不是人脸");
	}
	// printf("%d\n", IS_FACE);
	return  IS_FACE;
} 
