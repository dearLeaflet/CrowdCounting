#include <opencv2/opencv.hpp>
#include <fstream>
#include <random>
#include <sstream>
using namespace std;
using namespace cv;
struct PicLabel
{
	string imgName;
	string imgLocation;
	string imgROI;
};
void shuffle(string& PathName, int iteratorN)
{
	RNG rng;
	/*Size sizet(18, 18);*/
	ofstream fcrowdShuffle;
	fcrowdShuffle.open("../shuffle.txt");

	fstream f;
	f.open("../lableimage.txt");
	

	string line, location, roi;
	int frame = 0;
	vector<PicLabel> PicLabelV;
	while (getline(f, line)){
		getline(f, location);
		getline(f, roi);
		PicLabel p;
		p.imgName = line;
		p.imgLocation = location;
		p.imgROI = roi;
		PicLabelV.push_back(p);
	}

	int length = PicLabelV.size();
	int randomL = length - 1;
	while (iteratorN--){
		cout << "iterate shuffle " << iteratorN << " ´Î" << endl;
		for (int i = 0; i < length; i++){
			cout << "swapline ... " << i << " ´Î" << endl;
			int swapElement = rng.uniform(0, randomL);
			int swapElement1 = rng.uniform(0, randomL);
			swap(PicLabelV[swapElement], PicLabelV[swapElement1]);
		}
	}
	
	for (auto &i: PicLabelV){
		fcrowdShuffle << i.imgName << "\n";
		fcrowdShuffle << i.imgLocation << "\n";
		fcrowdShuffle << i.imgROI << "\n";
	}
	fcrowdShuffle.close();
}


int main()
{
	
	shuffle(string("../lableimage.txt"), 4);
	return 0;
}