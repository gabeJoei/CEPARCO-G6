
size_t calculate_barycenter_criteria_c(
    size_t features,
    size_t rows,
    float *X,
    float *own_barycenter,
    float *opposite_barycenter,
    float *filtered
);

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
