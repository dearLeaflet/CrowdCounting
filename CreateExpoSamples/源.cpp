#include <opencv2/opencv.hpp>
#include <fstream>
#include <sstream>
#include <time.h>
using namespace std;
using namespace cv;
#define WIDTH 720
#define HEIGHT 576

/*
ofstream f1;
Mat img;
RNG rng;
int preX, preY, deltaY;

double label[480][640];
string int2string(int n)
{
string name = "seq_000000";
string ints = to_string(n);
int nameLength = name.size() - 1;
for (int i = name.size() - ints.size(), j = 0; i < name.size(); ++i, ++j) { name[i] = ints[j]; }
return name + ".jpg";
}



int getDeltaY(int y)
{
return abs(y - preY);
}
void mouseClick(int event, int x, int y, int flags, void*)
{
switch (event)
{
case EVENT_LBUTTONDOWN:
preY = y;
preX = x;
break;
case EVENT_LBUTTONUP:
deltaY = getDeltaY(y);
cout << "locat: " << "(" << preX << ", " << preY << ")" << " = " << deltaY <<endl;
f1 << preX << " " << preY << " " << deltaY << "\n";
line(img, Point(preX, preY), Point(x, y), Scalar(0, 255, 255));

break;
}
}


Mat getPerspectiveMat(int a = 0.2965, int b = 47.613)
{
Mat img = Mat::zeros(Size(640, 480), CV_8UC1);
for (int i = 0; i < img.rows; i++)
{
for (int j = 0; j < img.cols; j++)
img.data[i*img.rows + j] = (a * j + b) / 175.0;
}
Mat imgColor;
cvtColor(img,imgColor, CV_GRAY2BGR);
cvtColor(imgColor, imgColor, CV_BGR2HSV);
imshow("HSV", imgColor);
return imgColor;
}
*/


bool rectRefine(Rect &rect, Mat& roiMask)
{
	/*if (rect.area() < 1296)
	return false;
	int w = rect.width;
	int h = rect.height;
	if (rect.x < 0)
	{
	rect.x = 0;
	rect.width = w;
	}
	if (rect.y < 0)
	{
	rect.y = 0;
	rect.height = h;
	}
	if (rect.br().x >= 720)
	{
	rect.x = 719 - w;
	rect.width = w;
	}
	if (rect.br().y >= 576)
	{
	rect.y = 575 - h;
	rect.height = h;
	}*/
	if (!(rect.x > 0 && rect.y > 0 && rect.br().x < 720 && rect.br().y < 576))
		return false;
	if (roiMask.at<uchar>(rect.y + rect.height / 2, rect.x + rect.width / 2) == 0)
		return false;
	if (rect.area() < 900)
		return false;
	return true;
}
float guassPD(float x, float sigma)
{
	return (1 / (sigma * sqrt(3.1415 * 2))) * exp(-0.5 * pow(x, 2) / pow(sigma, 2));
}

float guassBivaryPD(int xd, int yd, int u1, int u2, float sigmax, float sigmay)
{
	float p1 = 1 / (2 * 3.1415 *sigmax * sigmay);
	float p2 = -0.5 * (pow(xd - u1, 2) / pow(sigmax, 2) + pow(yd - u2, 2) / pow(sigmay, 2));
	return p1 * exp(p2);
}




int main()
{

	string data_path = "D:/Data/expo2010/";
	/*default_random_engine generator;
	uniform_int_distribution<int> distrbutionx(0, 720), distributiony(0, 576);*/
	RNG rng;
	Size size(72, 72);
	/*Size sizet(18, 18);*/
	ofstream fcrowd, fcrowdv;
	fcrowd.open(data_path + "train_samples/sampleCrowd18.txt");
	fcrowdv.open(data_path + "train_samples/sampleCrowdv18.txt");

	//ofstream fcrowd36, fcrowdv36;
	//fcrowd36.open("../sampleCrowd36.txt");
	//fcrowdv36.open("../sampleCrowdv36.txt");
	//
	//ofstream fcrowd72, fcrowdv72;
	//fcrowd72.open("../sampleCrowd72.txt");
	//fcrowdv72.open("../sampleCrowdv72.txt");


	ofstream fcount, fcountv;
	fcount.open(data_path + "train_samples/sampleCount.txt");
	fcountv.open(data_path + "train_samples/sampleCountV.txt");

	fstream f;
	f.open(data_path + "train_shuffle.txt");
	string line;
	int sampelNum = 0;
	int frame = 0;
	time_t start = clock();
	while (getline(f, line))
	{
		string location, roi;
		getline(f, location);
		getline(f, roi);
		if (true) {
			if (frame % 900 == 0) {
				fcrowd.close();
				waitKey(10);
				string file2open18 = data_path + "train_samples/sampleDensity18_" + to_string(frame / 900 + 1) + ".txt";
				fcrowd.open(file2open18);

				/*fcrowd36.close();
				waitKey(10);
				string file2open36 = "../sampletxt/sampleCrowd36_" + to_string(frame / 50 + 1) + ".txt";
				fcrowd36.open(file2open36);

				fcrowd72.close();
				waitKey(10);
				string file2open72 = "../sampletxt/sampleCrowd72_" + to_string(frame / 50 + 1) + ".txt";
				fcrowd72.open(file2open72);*/
			}

			if (frame % 8000 == 0) {
				fcrowdv.close();
				waitKey(10);
				string file2open18 = data_path + "train_samples/sampleDensityV18_" + to_string(frame / 8000 + 1) + ".txt";
				fcrowdv.open(file2open18);

				//fcrowdv36.close();
				//waitKey(10);
				//string file2open36 = "../sampletxt/sampleCrowdv36_" + to_string(frame / 900 + 1) + ".txt";
				//fcrowdv36.open(file2open36);

				//fcrowdv72.close();
				//waitKey(10);
				//string file2open72 = "../sampletxt/sampleCrowdv72_" + to_string(frame / 900 + 1) + ".txt";
				//fcrowdv72.open(file2open72);
			}




			stringstream names(line), locations(location);
			string nameframe, nameperspective;

			names >> nameframe >> nameperspective;

			/*
			*just test bug
			*/
			/*	line = "train_frame/200684_C09-02-S20100626083000000E20100626233000000_4_clip1_3.jpg	perspective/200684.jpg",
			location = "52	621.000000	492.000000	631.000000	557.000000	374.000000	505.000000	338.000000	494.000000	246.000000	549.000000	64.000000	538.000000	15.000000	491.000000	89.000000	451.000000	256.000000	402.000000	326.000000	395.000000	356.000000	404.000000	212.000000	358.000000	193.000000	322.000000	173.000000	324.000000	129.000000	316.000000	83.000000	366.000000	47.000000	371.000000	60.000000	281.000000	178.000000	270.000000	187.000000	254.000000	199.000000	255.000000	212.000000	267.000000	330.000000	292.000000	516.000000	255.000000	527.000000	252.000000	543.000000	261.000000	574.000000	261.000000	533.000000	363.000000	597.000000	360.000000	618.000000	331.000000	319.000000	297.000000	339.000000	303.000000	355.000000	290.000000	470.000000	264.000000	232.000000	237.000000	224.000000	236.000000	212.000000	231.000000	276.000000	229.000000	285.000000	226.000000	307.000000	225.000000	319.000000	226.000000	374.000000	218.000000	392.000000	221.000000	406.000000	223.000000	420.000000	220.000000	441.000000	217.000000	557.000000	222.000000	98.000000	243.000000	122.000000	234.000000	144.000000	222.000000	157.000000	218.000000	175.000000	222.000000",
			roi = "6	8.500000	292.112903	130.306452	246.693548	607.209677	247.725806	708.370968	569.790323	9.532258	572.887097	8.500000	292.112903";*/

			Mat roiMask = Mat::zeros(Size(WIDTH, HEIGHT), CV_8UC1);
			stringstream roistr(roi);
			int roiPoint;
			roistr >> roiPoint;
			vector<Point> roiPointV;
			for (int i = 0; i < roiPoint; ++i) {
				float x, y;
				roistr >> x >> y;
				int xx = x, yy = y;
				roiPointV.push_back(Point(xx, yy));

			}


			/*imshow("roim", roiMask);
			waitKey(10);*/
			Mat imgFrameS = imread(data_path + nameframe, CV_LOAD_IMAGE_COLOR);

			Mat imgPerspective = imread(data_path + nameperspective, CV_LOAD_IMAGE_GRAYSCALE);

			fillConvexPoly(roiMask, roiPointV, 255);
			//imshow("f", roiMask);
			if (!imgPerspective.empty() && !imgFrameS.empty())
			{
				Mat imgFrame = Mat::zeros(imgFrameS.size(), CV_8UC3);

				imgFrameS.copyTo(imgFrame, roiMask & 1);
				imgFrame.setTo(Scalar(255, 255, 255), (~roiMask) & 1);


				//计算density map
				int personNum;
				locations >> personNum;
				Mat densityMap = Mat::zeros(imgFrame.size(), CV_32FC1);
				int row = imgFrame.rows, col = imgFrame.cols;

				for (int i = 0; i < personNum; i++)
				{

					float x, y;
					locations >> x >> y;
					//cout << x << "," << y << endl;

					int bodyY = y + 0.8 * imgPerspective.at<uchar>(y, x);
					for (int m = 0; m < row; ++m)
						for (int n = 0; n < col; ++n)
						{
							uchar headDelta = imgPerspective.at<uchar>(m, n);
							if (bodyY < row - 1)
							{
								/*densityMap.at<float>(m, n) += ((guassBivaryPD(n, m, x, bodyY,
								imgPerspective.at<uchar>(bodyY, n) * 0.2, imgPerspective.at<uchar>(bodyY, n) * 0.5))
								+ guassPD(dist, imgPerspective.at<uchar>(m, n) * 0.2));*/

								/*densityMap.at<float>(m, n) += ((guassBivaryPD(n, m, x, bodyY,
								imgPerspective.at<uchar>(bodyY, n) * 0.2, imgPerspective.at<uchar>(bodyY, n) * 0.5))
								) * 1000;*/

								uchar bodyDelta = imgPerspective.at<uchar>(bodyY, n);


								densityMap.at<float>(m, n) += ((guassBivaryPD(n, m, x, bodyY,
									1.0 * bodyDelta * 0.2, 1.0 * bodyDelta * 0.5))
									+ guassPD(sqrt(pow(x - n, 2) + pow(y - m, 2)),
										headDelta * 0.2));
							}
							else
							{
								densityMap.at<float>(m, n) += guassPD(sqrt(pow(x - n, 2) + pow(y - m, 2)),
									0.2 * headDelta);
							}
						}

				}

				/*Mat iii = imgFrame.clone();

				cvtColor(iii, iii, CV_BGR2GRAY);
				for (int i = 0; i < densityMap.rows; ++i)
				for (int j = 0; j < densityMap.cols; ++j)
				if (densityMap.at<float>(i, j) > 1)
				iii.at<uchar>(i, j) += 120;
				imshow("iii", iii);*/

				// density归一化
				double pSum = 0;
				for (int i = 0; i < densityMap.rows; ++i) {
					float* data = densityMap.ptr<float>(i); { 
						for (int j = 0; j < densityMap.cols; ++j)
							pSum += data[j];
					}
				}


				if (pSum * personNum != 0) {
#pragma omp parallel for
					for (int i = 0; i < densityMap.rows; ++i) {
						float* data = densityMap.ptr<float>(i);
						for (int j = 0; j < densityMap.cols; ++j)
							data[j] = (data[j] / pSum) * personNum;
					}
				}
				else densityMap = Mat::zeros(imgFrame.size(), CV_32FC1);

				densityMap.setTo(0, (~roiMask) & 1);//除去非iROI区域

													//roi samples
													//获取样本

				int getsample = 0;
				for (int iX = 0; iX < 100; iX++)
				{

					int centerX, centerY;
					centerX = rng.uniform(1, WIDTH);
					centerY = rng.uniform(1, HEIGHT);

					int pixelN = imgPerspective.at<uchar>(centerY, centerX);
					Rect ROI;
					ROI.x = centerX - 1.5 * pixelN;
					ROI.y = centerY - 1.5 * pixelN;
					ROI.width = 3 * pixelN;
					ROI.height = 3 * pixelN;
					if (rectRefine(ROI, roiMask))
					{
						//rectangle(imgFrame, ROI, Scalar(0, 0, 255), 1);
						++getsample;
						//cout << getsample * 2 << endl;
						//cout << getsample << endl;
						if (getsample % 20 != 0)
						{
							Mat imgFrameSamples = imgFrame(ROI).clone();
							string frameSaMpleName = to_string(sampelNum);
							string sampleSource = data_path + "train_samples/RGBSamples/" + frameSaMpleName + ".jpg";
							resize(imgFrameSamples, imgFrameSamples, size);
							Mat denstiySamples = densityMap(ROI).clone();
							float samleCountLabel = 0;
							for (int ci = 0; ci < denstiySamples.rows; ++ci) {
								for (int cj = 0; cj < denstiySamples.cols; ++cj) {
									samleCountLabel += denstiySamples.at<float>(ci, cj);
								}
							}
							fcount << frameSaMpleName + ".jpg" + "\t" << samleCountLabel << "\n";
							/*cout << samleCountLabel << endl;
							imshow("11", imgFrameSamples);
							waitKey(0);*/
							//保存彩色图像样本
							imwrite(sampleSource, imgFrameSamples);

							//保存density map
							Mat densityMap18;
							resize(denstiySamples, densityMap18, Size(18, 18));

							/*	Mat densityMap36;
							resize(denstiySamples, densityMap36, Size(36, 36));

							Mat densityMap72;
							resize(denstiySamples, densityMap72, Size(72, 72));*/

							//保存图像序列
							fcrowd << frameSaMpleName + ".jpg" + "\t";
							for (int j = 0; j < 18; j++) {
								float* data = densityMap18.ptr<float>(j);
								for (int i = 0; i < 18; i++) {
									fcrowd << data[i] << "\t";
								}
							}
							fcrowd << "\n";

							/*fcrowd36 << frameSaMpleName + ".jpg" + "\t";
							for (int j = 0; j < 36; j++) {
							float* data = densityMap36.ptr<float>(j);
							for (int i = 0; i < 36; i++) {
							fcrowd36 << data[i] << "\t";
							}
							}
							fcrowd36 << "\n";

							fcrowd72 << frameSaMpleName + ".jpg" + "\t";
							for (int j = 0; j < 72; j++) {
							float* data = densityMap72.ptr<float>(j);
							for (int i = 0; i < 72; i++) {
							fcrowd72 << data[i] << "\t";
							}
							}
							fcrowd72 << "\n";*/

							/*
							*creat mirror samples
							*/
							Mat imgFrameSamplesM = imgFrame(ROI).clone();
							resize(imgFrameSamplesM, imgFrameSamplesM, size);
							flip(imgFrameSamplesM, imgFrameSamplesM, 2);
							++sampelNum;
							frameSaMpleName = to_string(sampelNum);
							sampleSource = data_path + "train_samples/RGBSamples/" + frameSaMpleName + ".jpg";

							//保存彩色图像样本
							imwrite(sampleSource, imgFrameSamplesM);

							//保存flip density map

							resize(denstiySamples, densityMap18, Size(18, 18));
							flip(densityMap18, densityMap18, 2);

							/*resize(denstiySamples, densityMap36, Size(36, 36));
							flip(densityMap36, densityMap36, 2);


							resize(denstiySamples, densityMap72, Size(72, 72));
							flip(densityMap72, densityMap72, 2);*/
							//保存图像序列
							fcrowd << frameSaMpleName + ".jpg" + "\t";
							for (int j = 0; j < 18; j++) {
								float* data = densityMap18.ptr<float>(j);
								for (int i = 0; i < 18; i++) {
									fcrowd << data[i] << "\t";
								}
							}
							fcrowd << "\n";

							/*fcrowd36 << frameSaMpleName + ".jpg" + "\t";
							for (int j = 0; j < 36; j++) {
							float* data = densityMap36.ptr<float>(j);
							for (int i = 0; i < 36; i++) {
							fcrowd36 << data[i] << "\t";
							}
							}
							fcrowd36 << "\n";

							fcrowd72 << frameSaMpleName + ".jpg" + "\t";
							for (int j = 0; j < 72; j++) {
							float* data = densityMap72.ptr<float>(j);
							for (int i = 0; i < 72; i++) {
							fcrowd72 << data[i] << "\t";
							}
							}
							fcrowd72 << "\n";*/

							fcount << frameSaMpleName + ".jpg" + "\t" << samleCountLabel << "\n";

							++sampelNum;
						}
						else
						{
							Mat imgFrameSamples = imgFrame(ROI).clone();
							string frameSaMpleName = to_string(sampelNum);
							string sampleSource = data_path + "train_samples/RGBSamples/" + frameSaMpleName + ".jpg";
							
							resize(imgFrameSamples, imgFrameSamples, size);
							Mat denstiySamples = densityMap(ROI).clone();
							float samleCountLabel = 0;
							for (int ci = 0; ci < denstiySamples.rows; ++ci) {
								float* data = denstiySamples.ptr<float>(ci);
								for (int cj = 0; cj < denstiySamples.cols; ++cj) {
									samleCountLabel += data[cj];
								}
							}
							fcountv << frameSaMpleName + ".jpg" + "\t" << samleCountLabel << "\n";

							//保存彩色图像样本
							imwrite(sampleSource, imgFrameSamples);

							//保存density map
							Mat densityMap18;
							resize(denstiySamples, densityMap18, Size(18, 18));

							//Mat densityMap36;
							//resize(denstiySamples, densityMap36, Size(36, 36));

							//Mat densityMap72;
							//resize(denstiySamples, densityMap72, Size(72, 72));

							//保存图像序列
							fcrowdv << frameSaMpleName + ".jpg" + "\t";
							for (int j = 0; j < 18; j++) {
								float* data = densityMap18.ptr<float>(j);
								for (int i = 0; i < 18; i++) {
									fcrowdv << data[i] << "\t";
								}
							}
							fcrowdv << "\n";

							/*fcrowdv36 << frameSaMpleName + ".jpg" + "\t";
							for (int j = 0; j < 36; j++) {
							float* data = densityMap36.ptr<float>(j);
							for (int i = 0; i < 36; i++) {
							fcrowdv36 << data[i] << "\t";
							}
							}
							fcrowdv36 << "\n";

							fcrowdv72 << frameSaMpleName + ".jpg" + "\t";
							for (int j = 0; j < 72; j++) {
							float* data = densityMap72.ptr<float>(j);
							for (int i = 0; i < 72; i++) {
							fcrowdv72 << data[i] << "\t";
							}
							}
							fcrowdv72 << "\n";*/

							/*
							*creat mirror samples
							*/
							Mat imgFrameSamplesM = imgFrame(ROI).clone();
							resize(imgFrameSamplesM, imgFrameSamplesM, size);
							flip(imgFrameSamplesM, imgFrameSamplesM, 2);
							++sampelNum;
							frameSaMpleName = to_string(sampelNum);
							sampleSource = data_path + "train_samples/RGBSamples/" + frameSaMpleName + ".jpg";


							//保存彩色图像样本
							imwrite(sampleSource, imgFrameSamplesM);


							//保存flip density map

							resize(denstiySamples, densityMap18, Size(18, 18));
							flip(densityMap18, densityMap18, 2);

							/*resize(denstiySamples, densityMap36, Size(36, 36));
							flip(densityMap36, densityMap36, 2);


							resize(denstiySamples, densityMap72, Size(72, 72));
							flip(densityMap72, densityMap72, 2);*/
							//保存图像序列
							fcrowdv << frameSaMpleName + ".jpg" + "\t";
							for (int j = 0; j < 18; j++) {
								float* data = densityMap18.ptr<float>(j);
								for (int i = 0; i < 18; i++) {
									fcrowdv << data[i] << "\t";
								}
							}
							fcrowdv << "\n";

							/*	fcrowdv36 << frameSaMpleName + ".jpg" + "\t";
							for (int j = 0; j < 36; j++) {
							float* data = densityMap36.ptr<float>(j);
							for (int i = 0; i < 36; i++) {
							fcrowdv36 << data[i] << "\t";
							}
							}
							fcrowdv36 << "\n";

							fcrowdv72 << frameSaMpleName + ".jpg" + "\t";
							for (int j = 0; j < 72; j++) {
							float* data = densityMap72.ptr<float>(j);
							for (int i = 0; i < 72; i++) {
							fcrowdv72 << data[i] << "\t";
							}
							}
							fcrowdv72 << "\n";*/

							fcountv << frameSaMpleName + ".jpg" + "\t" << samleCountLabel << "\n";

							++sampelNum;
						}

						//cout << sampelNum << endl;


					}
					//cout << getsample << "*********************************" << endl;



				}
				/*densityMap *= 10000;
				imwrite("dens.png", densityMap);
				imshow("densityMap", densityMap);
				imshow("imgFrame", imgFrame);
				waitKey(0);*/
			}
		}
		//imshow("aaa", imgFrame);
		waitKey(10);
		cout << frame << endl;
		++frame;
		cout << "spend: " << (clock() - start) / CLOCKS_PER_SEC << " S" << endl;
	}
	fcrowdv.close();
	fcrowd.close();
	//fcrowdv36.close();
	//fcrowd36.close();
	//fcrowdv72.close();
	//fcrowd72.close();

	fcount.close();
	fcountv.close();
	f.close();
	return 0;
}