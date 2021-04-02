#include "BackgroundSubtraction.h"
using namespace std;

int main (){

	BackgroundSubtraction vid("0");
	vid.removeBackground(50, 100, 99.0, 0.01);
	return 0;
}
