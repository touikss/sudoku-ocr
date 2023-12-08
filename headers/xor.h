#ifndef SUDOKUSOLVER_XOR_H
#define SUDOKUSOLVER_XOR_H

// we choose sigmoid function because this is the easyiest function to work with.
float sigmoid(float x);

float sigmoid_derivative(float x);

typedef struct Neuron {
    float weights[2];
    float bias;
} Neuron;


void init_neuron(Neuron *neuron);

//it return the value of the neuron which is the sum of all weights + the bias into the sigmoid function
float neuronOutput(Neuron *neuron, float inputs[2]);

void adjustWeights(Neuron *neuron, float error, float inputs[2]);

int Xor(int nb_tnb_iteration, int display_step);

#endif //SUDOKUSOLVER_XOR_H
