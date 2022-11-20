#include <math.h>

double ft_fabs(double x)
{
	if (x < 0) {
		return -x;
	}
	return x;
}

double ft_sqrt(double x)
{
	if (x < 0) {
		return -NAN;
	}
	double lower_bound = 0;
	double upper_bound = x;
	if (x < 1) {
		upper_bound = 1;
	}
	double sqrt = (lower_bound + upper_bound) / 2;
	double square = sqrt * sqrt;
	while (ft_fabs(square - x) > 0.0000000001) {
		if (square > x) {
			upper_bound = sqrt;
		} else {
			lower_bound = sqrt;
		}
		sqrt = (lower_bound + upper_bound) / 2;
		square = sqrt * sqrt;
	}
	return sqrt;
}
