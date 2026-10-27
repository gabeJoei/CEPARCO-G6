#include <stdio.h>
#include <math.h>
#include <stdbool.h>

typedef struct {
	float x;
	float y;
} Point;

float barycenter(float* x, size_t n);
bool barycenterCriteria(Point x, Point z1, Point z2);

int main() {
	//sample usage
	float y[3] = {1.0, 2.0, 3.5};
	printf("barycenter calculation for y data points = %f\n", barycenterCalculation(y,3));

	Point x = {1,0};
	Point z1 = {3,0};
	Point z2 = {4,0};
	if(barycenterCriteria(x,z1,z2))
		printf("passes the criteria!\n");	
}

/**
 * @brief Calculates the barycenter (arithmetic mean) of a set of values.
 *
 * @param x Pointer to an array of floating-point values.
 * @param n Number of elements in the array.
 * @return The arithmetic mean of the values.
 */
float barycenter(float* x, size_t n) {
	float sum = 0;
	for(int i=0; i < n; i++) { 
		sum += x[i];
	}
	return sum * (1/(n*1.0f));
}

/**
 * @brief Evaluates whether a point satisfies the barycenter filtering criterion.
 *
 * Compares the Euclidean distance between point x and its
 * original class barycenter z1 against the distance between
 * the two class barycenters z1 and z2.
 *
 * @param x  Point being evaluated.
 * @param z1 Barycenter of the original class.
 * @param z2 Barycenter of the other class.
 * @return true if distance(x, z1) > distance(z1, z2);
 *         false otherwise.
 */
bool barycenterCriteria(Point x, Point z1, Point z2) {
	float pointAgainstCluster = sqrt(pow((x.x-z1.x),2)+pow((x.y-z1.y),2));
	float clusterAgainstCluster = sqrt(pow((z1.x-z2.x),2)+pow((z1.y-z2.y),2)); 
	if(pointAgainstCluster > clusterAgainstCluster)	
		return true;
	return false;
}
