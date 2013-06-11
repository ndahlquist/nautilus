#ifndef NOISE_H_
#define NOISE_H_


double composedNoise(double x, double y, double z, int octaves, int p, int baseFreq);
void initNoise(float seed);


#endif /*NOISE_H_*/