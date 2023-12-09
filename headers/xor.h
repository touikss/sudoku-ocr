#ifndef SUDOKUSOLVER_XOR_H
#define SUDOKUSOLVER_XOR_H


float sigmoid(float x);

float sigmoid_derivative(float x);

typedef struct Neuron {
    float weights[2];
    float bias;
} Neuron;


void init_neuron(Neuron *neuron);


float neuronOutput(Neuron *neuron, float inputs[2]);

void adjustWeights(Neuron *neuron, float error, float inputs[2]);

int Xor(int nb_tnb_iteration, int display_step);

#endif 
