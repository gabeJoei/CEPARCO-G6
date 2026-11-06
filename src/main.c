#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ROWS 1024 // adjust as needed. The csv is 815x5
#define NUM_FEATURES 5

void compute_barycenter(size_t features, size_t rows, float **X, float *out_barycenter);

int main()
{
	// STEP 0: Initialize memory space for features x1-x5 as rows
	float **X = malloc(MAX_ROWS * sizeof(float *));
	for (int i = 0; i < MAX_ROWS; i++)
	{
		X[i] = malloc(NUM_FEATURES * sizeof(float));
	}
	size_t *y = malloc(MAX_ROWS * sizeof(size_t));

	// STEP 1: Read from csv
	FILE *file = fopen("../data.csv", "r");
	if (!file)
	{
		// Clean up before exiting
		for (int i = 0; i < MAX_ROWS; i++)
			free(X[i]);
		free(X);
		free(y);
		return 1; // indicate error
	}

	char line[MAX_ROWS];
	// skip header by reading first line
	fgets(line, sizeof(line), file);

	size_t row = 0;
	while (fgets(line, sizeof(line), file) && row < MAX_ROWS)
	{
		// remove trailing \n
		line[strcspn(line, "\n")] = 0;
		size_t featureCol = 0;

		char *token = strtok(line, ","); // Get first token @ featureCol 0
		while (token != NULL)
		{
			if (featureCol < NUM_FEATURES)
			{
				// Store x1..x5 directly into X[row][col]
				X[row][featureCol] = (float)atof(token);
			}
			else if (featureCol == NUM_FEATURES)
			{
				// Store y in array
				y[row] = atoi(token);
			}
			token = strtok(NULL, ",");
			featureCol++;
		}
		row++;
	}
	fclose(file);

	// Correctness check of data extraction
	printf("======== DATA EXTRACTION PREVIEW ========\n");
	for (size_t i = 0; i < 5; i++) // only until feature x2 im too lazy to expand
		printf("Row %zu: x1 = %.2f, x2 = %.2f\n", i, X[i][0], X[i][1]);

	// Example barycenter calculation
	float z_B1[NUM_FEATURES];
	compute_barycenter(NUM_FEATURES, row, X, z_B1);

	printf("\nBarycenter Vector:\n[ ");
	for (size_t i = 0; i < NUM_FEATURES; i++)
	{
		printf("%.4f ", z_B1[i]);
	}
	printf("]\n");

	// STEP #: Free heap memory
	for (int i = 0; i < MAX_ROWS; i++)
		free(X[i]);
	free(X);
	free(y);

	return 0;
}

// returns a barycenter (5D vector) of a single datacluster/class
void compute_barycenter(size_t features, size_t rows, float **X, float *out_barycenter)
{
	for (size_t i = 0; i < features; i++)
	{
		float featureSum = 0.0f;
		for (size_t j = 0; j < rows; j++)
		{
			// compute sum of feature
			featureSum += X[j][i];
		}
		out_barycenter[i] = featureSum / (float)rows;
	}
}