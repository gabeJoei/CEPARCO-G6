#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ROWS 1024 // adjust as needed. The csv is 815x5
#define NUM_FEATURES 5

void compute_barycenter(size_t features, size_t rows, float *X, float *out_barycenter);

int main()
{
	// STEP 0: Initialize memory space for classes w/ features x1-x5 as rows
	float *X_classNeg1 = malloc(MAX_ROWS * NUM_FEATURES * sizeof(float));
	float *X_class1 = malloc(MAX_ROWS * NUM_FEATURES * sizeof(float));

	// STEP 1: Read from csv
	FILE *file = fopen("../data.csv", "r");
	if (!file)
	{
		// Clean up before exiting
		free(X_classNeg1);
		free(X_class1);
		return 1; // indicate error
	}

	char line[MAX_ROWS];
	// skip header by reading first line
	fgets(line, sizeof(line), file);

	// size_t row = 0;
	int count_neg1 = 0; // # of rows where y == -1
	int count_1 = 0;	// # of rows where y == 1

	while (fgets(line, sizeof(line), file))
	{
		// remove trailing \n
		line[strcspn(line, "\r\n")] = 0;

		// to store unclassified rows
		float temp_features[NUM_FEATURES];

		int label = -1;
		size_t featureCol = 0;

		char *token = strtok(line, ","); // Get first token @ featureCol 0
		while (token != NULL)
		{
			if (featureCol < NUM_FEATURES)
			{
				// store all unclassified rows
				temp_features[featureCol] = (float)atof(token);
			}
			else if (featureCol == NUM_FEATURES)
			{
				label = atoi(token); // 6th column is y
			}
			token = strtok(NULL, ","); // move to next row
			featureCol++;
		}

		// move row into class
		if (label == -1)
		{
			for (int j = 0; j < NUM_FEATURES; j++)
			{
				X_classNeg1[count_neg1 * NUM_FEATURES + j] = temp_features[j];
			}
			count_neg1++;
		}
		else if (label == 1)
		{
			for (int j = 0; j < NUM_FEATURES; j++)
			{
				X_class1[count_1 * NUM_FEATURES + j] = temp_features[j];
			}
			count_1++;
		}
	}
	fclose(file);

	// Correctness check of data extraction
	printf("======== DATA EXTRACTION PREVIEW ========\n");
	printf("Extracted %d samples for Class 0\n", count_neg1);
	printf("Extracted %d samples for Class 1\n", count_1);
	printf("CLASS -1 (First 5 rows):\n");
	for (size_t i = 0; i < 5; i++)
	{
		// [i * NUM_FEATURES + col]
		printf("Row %zu: x1 = %.2f, x2 = %.2f\n",
			   i,
			   X_classNeg1[i * NUM_FEATURES + 0],
			   X_classNeg1[i * NUM_FEATURES + 1]);
	}
	printf("\nCLASS 1 (First 5 rows):\n");
	for (size_t i = 0; i < 5; i++)
	{
		// [i * NUM_FEATURES + col]
		printf("Row %zu: x1 = %.2f, x2 = %.2f\n",
			   i,
			   X_class1[i * NUM_FEATURES + 0],
			   X_class1[i * NUM_FEATURES + 1]);
	}

	// Example barycenter calculation for both classes
	float z_B_neg1[NUM_FEATURES];
	float z_B_pos1[NUM_FEATURES];

	if (count_neg1 > 0)
	{
		compute_barycenter(NUM_FEATURES, count_neg1, X_classNeg1, z_B_neg1);
		printf("\nBarycenter Vector (Class -1):\n[ ");
		for (size_t i = 0; i < NUM_FEATURES; i++)
		{
			printf("%.4f ", z_B_neg1[i]);
		}
		printf("]\n");
	}

	if (count_1 > 0)
	{
		compute_barycenter(NUM_FEATURES, count_1, X_class1, z_B_pos1);
		printf("\nBarycenter Vector (Class 1):\n[ ");
		for (size_t i = 0; i < NUM_FEATURES; i++)
		{
			printf("%.4f ", z_B_pos1[i]);
		}
		printf("]\n");
	}

	// STEP #: Free heap memory
	free(X_classNeg1);
	free(X_class1);

	return 0;
}

// returns a barycenter (5D vector) of a single datacluster/class
void compute_barycenter(size_t features, size_t rows, float *X, float *out_barycenter)
{
	for (size_t i = 0; i < features; i++)
	{
		float featureSum = 0.0f;
		for (size_t j = 0; j < rows; j++)
		{
			// compute sum of feature
			featureSum += X[j * features + i];
		}
		out_barycenter[i] = featureSum / (float)rows;
	}
}